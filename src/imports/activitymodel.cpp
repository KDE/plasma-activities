/*
    SPDX-FileCopyrightText: 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Self
#include "activitymodel.h"

// Qt
#include <QByteArray>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include <QFutureWatcher>
#include <QHash>
#include <QIcon>
#include <QList>
#include <QModelIndex>

// KDE
#include <KConfig>
#include <KConfigGroup>
#include <KDirWatch>

#include <optional>

// Local
#define ENABLE_QJSVALUE_CONTINUATION
#include "utils/continue_with.h"
#include "utils/model_updaters.h"

using kamd::utils::continue_with;

namespace KActivities
{
namespace Imports
{
class ActivityModel::Private
{
public:
    DECLARE_RAII_MODEL_UPDATERS(ActivityModel)

    /**
     * Searches for the activity.
     * Returns an option(index, iterator) for the found activity.
     */
    template<typename _Container>
    static inline std::optional<std::pair<unsigned int, typename _Container::const_iterator>> activityPosition(const _Container &container,
                                                                                                               const QString &activityId)
    {
        using ActivityPosition = decltype(activityPosition(container, activityId));
        using ContainerElement = typename _Container::value_type;

        auto position = std::find_if(container.begin(), container.end(), [&](const ContainerElement &activity) {
            return activity->id() == activityId;
        });

        return (position != container.end()) ? ActivityPosition(std::make_pair(position - container.begin(), position)) : ActivityPosition();
    }

    /**
     * Notifies the model that an activity was updated
     */
    template<typename _Model, typename _Container>
    static inline void emitActivityUpdated(_Model *model, const _Container &container, QObject *activityInfo, int role)
    {
        const auto activity = static_cast<Info *>(activityInfo);
        emitActivityUpdated(model, container, activity->id(), role);
    }

    /**
     * Notifies the model that an activity was updated
     */
    template<typename _Model, typename _Container>
    static inline void emitActivityUpdated(_Model *model, const _Container &container, const QString &activity, int role)
    {
        auto position = Private::activityPosition(container, activity);

        if (position) {
            Q_EMIT model->dataChanged(model->index(position->first),
                                      model->index(position->first),
                                      role == Qt::DecorationRole ? QList<int>{role, ActivityModel::ActivityIcon} : QList<int>{role});
        }
    }

    class BackgroundCache
    {
    public:
        BackgroundCache()
            : initialized(false)
            , plasmaConfig(QStringLiteral("plasma-org.kde.plasma.desktop-appletsrc"))
        {
            using namespace std::placeholders;

            const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + plasmaConfig.name();

            KDirWatch::self()->addFile(configFile);

            connect(KDirWatch::self(), &KDirWatch::dirty, std::bind(&BackgroundCache::settingsFileChanged, this, _1));
            connect(KDirWatch::self(), &KDirWatch::created, std::bind(&BackgroundCache::settingsFileChanged, this, _1));
        }

        void settingsFileChanged(const QString &file)
        {
            if (!file.endsWith(plasmaConfig.name())) {
                return;
            }

            plasmaConfig.reparseConfiguration();

            if (initialized) {
                reload(false);
            }
        }

        void subscribe(ActivityModel *model)
        {
            if (!initialized) {
                reload(true);
            }

            models << model;
        }

        void unsubscribe(ActivityModel *model)
        {
            models.removeAll(model);

            if (models.isEmpty()) {
                initialized = false;
                forActivity.clear();
            }
        }

        QString backgroundFromConfig(const KConfigGroup &config) const
        {
            auto wallpaperPlugin = config.readEntry("wallpaperplugin");
            auto wallpaperConfig = config.group(QStringLiteral("Wallpaper")).group(wallpaperPlugin).group(QStringLiteral("General"));

            if (wallpaperConfig.hasKey("Image")) {
                // Trying for the wallpaper
                auto wallpaper = wallpaperConfig.readEntry("Image", QString());
                if (!wallpaper.isEmpty()) {
                    return wallpaper;
                }
            }
            if (wallpaperConfig.hasKey("Color")) {
                auto backgroundColor = wallpaperConfig.readEntry("Color", QColor(0, 0, 0));
                return backgroundColor.name();
            }

            return QString();
        }

        void reload(bool fullReload)
        {
            QHash<QString, QString> newBackgrounds;

            if (fullReload) {
                forActivity.clear();
            }

            QStringList changedBackgrounds;

            for (const auto &cont : plasmaConfigContainments().groupList()) {
                auto config = plasmaConfigContainments().group(cont);
                auto activityId = config.readEntry("activityId", QString());

                // Ignore if it has no assigned activity
                if (activityId.isEmpty()) {
                    continue;
                }

                // Ignore if we have already found the background
                if (newBackgrounds.contains(activityId) && newBackgrounds[activityId][0] != QLatin1Char('#')) {
                    continue;
                }

                auto newBackground = backgroundFromConfig(config);

                if (forActivity[activityId] != newBackground) {
                    changedBackgrounds << activityId;
                    if (!newBackground.isEmpty()) {
                        newBackgrounds[activityId] = newBackground;
                    }
                }
            }

            initialized = true;

            if (!changedBackgrounds.isEmpty()) {
                forActivity = newBackgrounds;

                for (auto model : models) {
                    model->backgroundsUpdated(changedBackgrounds);
                }
            }
        }

        KConfigGroup plasmaConfigContainments()
        {
            return plasmaConfig.group(QStringLiteral("Containments"));
        }

        QHash<QString, QString> forActivity;
        QList<ActivityModel *> models;

        bool initialized;
        KConfig plasmaConfig;
    };

    static BackgroundCache &backgrounds()
    {
        // If you convert this to a shared pointer,
        // fix the connections to KDirWatcher
        static BackgroundCache cache;
        return cache;
    }
};

ActivityModel::ActivityModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Initializing role names for qml
    connect(&m_service, &Consumer::serviceStatusChanged, this, &ActivityModel::setServiceStatus);

    connect(&m_service, &KActivities::Consumer::activityAdded, this, [this](const QString &id) {
        onActivityAdded(id);
    });
    connect(&m_service, &KActivities::Consumer::activityRemoved, this, &ActivityModel::onActivityRemoved);
    connect(&m_service, &KActivities::Consumer::currentActivityChanged, this, &ActivityModel::onCurrentActivityChanged);

    setServiceStatus(m_service.serviceStatus());

    Private::backgrounds().subscribe(this);
}

ActivityModel::~ActivityModel()
{
    Private::backgrounds().unsubscribe(this);
}

QHash<int, QByteArray> ActivityModel::roleNames() const
{
    return {{Qt::DisplayRole, "name"},
            {Qt::DecorationRole, "icon"},

            {ActivityId, "id"},
            {ActivityIcon, "iconSource"},
            {ActivityDescription, "description"},
            {ActivityBackground, "background"},
            {ActivityCurrent, "current"}};
}

void ActivityModel::setServiceStatus(Consumer::ServiceStatus)
{
    replaceActivities(m_service.activities());
}

void ActivityModel::replaceActivities(const QStringList &activities)
{
    // qDebug() << m_shownStatesString << "New list of activities: "
    //          << activities;
    // qDebug() << m_shownStatesString << " -- RESET MODEL -- ";

    Private::model_reset m(this);

    m_knownActivities.clear();
    m_shownActivities.clear();

    for (const QString &activity : activities) {
        onActivityAdded(activity, false);
    }
}

void ActivityModel::onActivityAdded(const QString &id, bool notifyClients)
{
    auto info = registerActivity(id);

    // qDebug() << m_shownStatesString << "Added a new activity:" << info->id()
    //          << " " << info->name();

    showActivity(info, notifyClients);
}

void ActivityModel::onActivityRemoved(const QString &id)
{
    // qDebug() << m_shownStatesString << "Removed an activity:" << id;

    hideActivity(id);
    unregisterActivity(id);
}

void ActivityModel::onCurrentActivityChanged(const QString &id)
{
    Q_UNUSED(id);

    for (const auto &activity : m_shownActivities) {
        Private::emitActivityUpdated(this, m_shownActivities, activity->id(), ActivityCurrent);
    }
}

ActivityModel::InfoPtr ActivityModel::registerActivity(const QString &id)
{
    auto position = Private::activityPosition(m_knownActivities, id);

    // qDebug() << m_shownStatesString << "Registering activity: " << id
    //          << " new? not " << (bool)position;

    if (position) {
        return *(position->second);

    } else {
        auto activityInfo = std::make_shared<Info>(id);

        auto ptr = activityInfo.get();

        connect(ptr, &Info::nameChanged, this, &ActivityModel::onActivityNameChanged);
        connect(ptr, &Info::descriptionChanged, this, &ActivityModel::onActivityDescriptionChanged);
        connect(ptr, &Info::iconChanged, this, &ActivityModel::onActivityIconChanged);

        m_knownActivities.insert(InfoPtr(activityInfo));

        return activityInfo;
    }
}

void ActivityModel::unregisterActivity(const QString &id)
{
    // qDebug() << m_shownStatesString << "Deregistering activity: " << id;

    auto position = Private::activityPosition(m_knownActivities, id);

    if (position) {
        if (auto shown = Private::activityPosition(m_shownActivities, id)) {
            Private::model_remove(this, QModelIndex(), shown->first, shown->first);
            m_shownActivities.erase(shown->second);
        }

        m_knownActivities.erase(position->second);
    }
}

void ActivityModel::showActivity(InfoPtr activityInfo, bool notifyClients)
{
    // Is it already shown?
    if (std::binary_search(m_shownActivities.begin(), m_shownActivities.end(), activityInfo, InfoPtrComparator())) {
        return;
    }

    auto registeredPosition = Private::activityPosition(m_knownActivities, activityInfo->id());

    if (!registeredPosition) {
        qDebug() << "Got a request to show an unknown activity, ignoring";
        return;
    }

    auto activityInfoPtr = *(registeredPosition->second);

    // qDebug() << m_shownStatesString << "Setting activity visibility to true:"
    //     << activityInfoPtr->id() << activityInfoPtr->name();

    auto [insertedIt, _, didInsert] = m_shownActivities.insert(activityInfoPtr);

    if (notifyClients) {
        unsigned int index = (didInsert ? insertedIt : m_shownActivities.end()) - m_shownActivities.begin();

        // qDebug() << m_shownStatesString << " -- MODEL INSERT -- " << index;
        Private::model_insert(this, QModelIndex(), index, index);
    }
}

void ActivityModel::hideActivity(const QString &id)
{
    auto position = Private::activityPosition(m_shownActivities, id);

    // qDebug() << m_shownStatesString
    //          << "Setting activity visibility to false: " << id;

    if (position) {
        // qDebug() << m_shownStatesString << " -- MODEL REMOVE -- "
        //          << position->first;
        Private::model_remove(this, QModelIndex(), position->first, position->first);
        m_shownActivities.erase(position->second);
    }
}
// clang-format off
#define CREATE_SIGNAL_EMITTER(What,Role)                                      \
    void ActivityModel::onActivity##What##Changed(const QString &)             \
    {                                                                          \
        Private::emitActivityUpdated(this, m_shownActivities, sender(), Role); \
    }
// clang-format on

CREATE_SIGNAL_EMITTER(Name, Qt::DisplayRole)
CREATE_SIGNAL_EMITTER(Description, ActivityDescription)
CREATE_SIGNAL_EMITTER(Icon, Qt::DecorationRole)

#undef CREATE_SIGNAL_EMITTER

void ActivityModel::backgroundsUpdated(const QStringList &activities)
{
    for (const auto &activity : activities) {
        Private::emitActivityUpdated(this, m_shownActivities, activity, ActivityBackground);
    }
}

int ActivityModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_shownActivities.size();
}

QVariant ActivityModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const auto &item = *(m_shownActivities.cbegin() + row);

    switch (role) {
    case Qt::DisplayRole:
        return item->name();

    case Qt::DecorationRole:
        return QIcon::fromTheme(data(index, ActivityIcon).toString());

    case ActivityId:
        return item->id();

    case ActivityIcon: {
        const QString &icon = item->icon();

        // We need a default icon for activities
        return icon.isEmpty() ? QStringLiteral("activities") : icon;
    }

    case ActivityDescription:
        return item->description();

    case ActivityCurrent:
        return m_service.currentActivity() == item->id();

    case ActivityBackground:
        return Private::backgrounds().forActivity[item->id()];

    default:
        return QVariant();
    }
}

QVariant ActivityModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);

    return QVariant();
}

ActivityModel::InfoPtr ActivityModel::findActivity(QObject *ptr) const
{
    auto info = std::find_if(m_knownActivities.begin(), m_knownActivities.end(), [ptr](const InfoPtr &info) {
        return ptr == info.get();
    });

    if (info == m_knownActivities.end()) {
        return nullptr;
    } else {
        return *info;
    }
}

// clang-format off
// QFuture<void> Controller::setActivityWhat(id, value)
#define CREATE_SETTER(What)                                                    \
    void ActivityModel::setActivity##What(                                     \
        const QString &id, const QString &value, const QJSValue &callback)     \
    {                                                                          \
        continue_with(m_service.setActivity##What(id, value), callback);       \
    }
// clang-format on

CREATE_SETTER(Name)
CREATE_SETTER(Description)
CREATE_SETTER(Icon)

#undef CREATE_SETTER

// QFuture<bool> Controller::setCurrentActivity(id)
void ActivityModel::setCurrentActivity(const QString &id, const QJSValue &callback)
{
    continue_with(m_service.setCurrentActivity(id), callback);
}

// QFuture<QString> Controller::addActivity(name)
void ActivityModel::addActivity(const QString &name, const QJSValue &callback)
{
    continue_with(m_service.addActivity(name), callback);
}

// QFuture<void> Controller::removeActivity(id)
void ActivityModel::removeActivity(const QString &id, const QJSValue &callback)
{
    continue_with(m_service.removeActivity(id), callback);
}

} // namespace Imports
} // namespace KActivities

#include "moc_activitymodel.cpp"
