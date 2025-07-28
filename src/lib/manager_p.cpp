/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "manager_p.h"

#include <mutex>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QFutureWatcher>
#include <QFutureWatcherBase>

#include "debug_p.h"
#include "mainthreadexecutor_p.h"

#include "common/dbus/common.h"
#include "utils/dbusfuture_p.h"
#include "version.h"

namespace KActivities
{
Manager *Manager::s_instance = nullptr;

Manager::Manager()
    : QObject()
    , m_watcher(KAMD_DBUS_SERVICE, QDBusConnection::sessionBus())
    , m_service(new KAMD_DBUS_CLASS_INTERFACE("/", Application, this))
    , m_activities(new KAMD_DBUS_CLASS_INTERFACE("Activities", Activities, this))
    , m_resources(new KAMD_DBUS_CLASS_INTERFACE("Resources", Resources, this))
    , m_resourcesLinking(new KAMD_DBUS_CLASS_INTERFACE("Resources/Linking", ResourcesLinking, this))
    , m_features(new KAMD_DBUS_CLASS_INTERFACE("Features", Features, this))
    , m_serviceStatus(Manager::NotRunning)
{
    connect(&m_watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &Manager::serviceOwnerChanged);

    if (serviceStatus()) {
        serviceOwnerChanged(KAMD_DBUS_SERVICE, QString(), KAMD_DBUS_SERVICE);
    }
}

Manager *Manager::self()
{
    static std::mutex singleton;
    std::lock_guard<std::mutex> singleton_lock(singleton);

    if (!s_instance) {
        runInMainThread([]() {
            // check if the activity manager is already running
            // creating a new instance of the class
            Manager::s_instance = new Manager();

            if (QDBusConnection::sessionBus().interface()->isServiceRegistered(KAMD_DBUS_SERVICE)) { // already running
                s_instance->m_serviceStatus = Running;
            } else { // not running
                bool disableAutolaunch = QCoreApplication::instance()->property("org.kde.KActivities.core.disableAutostart").toBool();

                qCDebug(KAMD_CORELIB) << "Should we start the daemon?";
                // start only if not disabled and we have a dbus connection at all
                if (!disableAutolaunch && QDBusConnection::sessionBus().interface()) {
                    qCDebug(KAMD_CORELIB) << "Starting the activity manager daemon";
                    auto busInterface = QDBusConnection::sessionBus().interface();
                    auto pending = busInterface->asyncCall(QStringLiteral("StartServiceByName"), KAMD_DBUS_SERVICE, uint(0));
                    s_instance->m_serviceStatus = Manager::Starting;
                    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pending, s_instance);
                    connect(watcher, &QDBusPendingCallWatcher::finished, watcher, []() {
                        const bool isRunning = QDBusConnection::sessionBus().interface()->isServiceRegistered(KAMD_DBUS_SERVICE);
                        s_instance->m_serviceStatus = isRunning ? Manager::Running : Manager::NotRunning;
                        Q_EMIT s_instance->serviceStatusChanged(s_instance->m_serviceStatus);
                    });
                }
            }
        });
    }
    return s_instance;
}

bool Manager::isServiceRunning()
{
    return s_instance && s_instance->m_serviceStatus == Manager::Running;
}

Manager::ServiceStatus Manager::serviceStatus()
{
    return s_instance ? s_instance->m_serviceStatus : NotRunning;
}

void Manager::serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner);
    Q_UNUSED(newOwner);

    if (serviceName == KAMD_DBUS_SERVICE) {
        const bool isRunning = QDBusConnection::sessionBus().interface()->isServiceRegistered(KAMD_DBUS_SERVICE);
        m_serviceStatus = isRunning ? Manager::Running : Manager::NotRunning;
        Q_EMIT serviceStatusChanged(m_serviceStatus);

        if (isRunning) {
            DBusFuture::fromReply(m_service->serviceVersion()).then([](const QString &serviceVersion) {
                // Test whether the service is older than the library.
                // If it is, we need to end this
                auto split = serviceVersion.split(QLatin1Char('.'));
                QList<int> version;

                // We require kactivitymanagerd version to be at least the
                // one before the repository split
                const int requiredVersion[] = {6, 2, 0};

                std::transform(split.cbegin(), split.cend(), std::back_inserter(version), [](const QString &component) {
                    return component.toInt();
                });

                // if required version is greater than the current version
                if (std::lexicographical_compare(version.cbegin(), version.cend(), std::begin(requiredVersion), std::end(requiredVersion))) {
                    QString libraryVersion = QString::number(requiredVersion[0]) + QLatin1Char('.') + QString::number(requiredVersion[1]) + QLatin1Char('.')
                        + QString::number(requiredVersion[2]);

                    qDebug() << "KActivities service version: " << serviceVersion;
                    qDebug() << "KActivities library version: " << libraryVersion;
                    qFatal("KActivities: FATAL ERROR: The service is older than the library");
                }
            });
        }
    }
}

Service::Activities *Manager::activities()
{
    return self()->m_activities;
}

Service::Resources *Manager::resources()
{
    return self()->m_resources;
}

Service::ResourcesLinking *Manager::resourcesLinking()
{
    return self()->m_resourcesLinking;
}

Service::Features *Manager::features()
{
    return self()->m_features;
}

} // namespace KActivities

#include "moc_manager_p.cpp"
