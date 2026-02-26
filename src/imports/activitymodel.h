/*
    SPDX-FileCopyrightText: 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KACTIVITIES_IMPORTS_ACTIVITY_MODEL_H
#define KACTIVITIES_IMPORTS_ACTIVITY_MODEL_H

// Qt
#include <QAbstractListModel>
#include <QCollator>
#include <QJSValue>
#include <QObject>
#include <qqmlregistration.h>

// STL
#include <memory>

// Local
#include "utils/qflatset.h"
#include <lib/consumer.h>
#include <lib/controller.h>
#include <lib/info.h>

class QModelIndex;
class QDBusPendingCallWatcher;

namespace KActivities
{
namespace Imports
{

/*!
 * \qmltype ActivityModel
 * \inqmlmodule org.kde.activities
 *
 * \brief Activities model.
 *
 * This model provides the following roles:
 * \list
 * \li name (string)
 * \li icon (QIcon)
 * \li id (string)
 * \li iconSource (string)
 * \li description (string)
 * \li background (string)
 * \li current (bool)
 * \endlist
 */
class ActivityModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ActivityModel(QObject *parent = nullptr);
    ~ActivityModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    enum Roles {
        ActivityId = Qt::UserRole,
        ActivityDescription = Qt::UserRole + 1,
        ActivityIcon = Qt::UserRole + 2,
        ActivityBackground = Qt::UserRole + 4,
        ActivityCurrent = Qt::UserRole + 5,
    };

public Q_SLOTS:
    // Activity control methods

    /*!
     * \qmlmethod void ActivityModel::setActivityName(string id, string name, var callback)
     */
    void setActivityName(const QString &id, const QString &name, const QJSValue &callback);

    /*!
     * \qmlmethod void ActivityModel::setActivityDescription(string id, string description, var callback)
     */
    void setActivityDescription(const QString &id, const QString &description, const QJSValue &callback);

    /*!
     * \qmlmethod void ActivityModel::setActivityIcon(string id, string icon, var callback)
     */
    void setActivityIcon(const QString &id, const QString &icon, const QJSValue &callback);

    /*!
     * \qmlmethod void ActivityModel::setCurrentActivity(string id, var callback)
     */
    void setCurrentActivity(const QString &id, const QJSValue &callback);

    /*!
     * \qmlmethod void ActivityModel::addActivity(string name, var callback)
     */
    void addActivity(const QString &name, const QJSValue &callback);

    /*!
     * \qmlmethod void ActivityModel::removeActivity(string id,  var callback)
     */
    void removeActivity(const QString &id, const QJSValue &callback);

Q_SIGNALS:
    /*!
     * \qmlsignal ActivityModel::shownStatesChanged(string state)
     */
    void shownStatesChanged(const QString &state);

private Q_SLOTS:
    void onActivityNameChanged(const QString &name);
    void onActivityDescriptionChanged(const QString &description);
    void onActivityIconChanged(const QString &icon);

    void replaceActivities(const QStringList &activities);
    void onActivityAdded(const QString &id, bool notifyClients = true);
    void onActivityRemoved(const QString &id);
    void onCurrentActivityChanged(const QString &id);

    void setServiceStatus(KActivities::Consumer::ServiceStatus status);

private:
    KActivities::Controller m_service;

    typedef std::shared_ptr<Info> InfoPtr;

    struct InfoPtrComparator {
        bool operator()(const InfoPtr &left, const InfoPtr &right) const
        {
            QCollator c;
            c.setCaseSensitivity(Qt::CaseInsensitive);
            c.setNumericMode(true);
            int rc = c.compare(left->name(), right->name());
            if (rc == 0) {
                return left->id() < right->id();
            }
            return rc < 0;
        }
    };

    QFlatSet<InfoPtr, InfoPtrComparator> m_knownActivities;
    QFlatSet<InfoPtr, InfoPtrComparator> m_shownActivities;

    InfoPtr registerActivity(const QString &id);
    void unregisterActivity(const QString &id);
    void showActivity(InfoPtr activityInfo, bool notifyClients);
    void hideActivity(const QString &id);
    void backgroundsUpdated(const QStringList &activities);

    InfoPtr findActivity(QObject *ptr) const;

    class Private;
    friend class Private;
};

} // namespace Imports
} // namespace KActivities

#endif // KACTIVITIES_IMPORTS_ACTIVITY_MODEL_H
