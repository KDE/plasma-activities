/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "consumer.h"
#include "consumer_p.h"
#include "manager_p.h"

#include "debug_p.h"

namespace KActivities
{
ConsumerPrivate::ConsumerPrivate()
    : cache(ActivitiesCache::self())
{
}

void ConsumerPrivate::setServiceStatus(Consumer::ServiceStatus status)
{
    Q_EMIT serviceStatusChanged(status);
}

Consumer::Consumer(QObject *parent)
    : QObject(parent)
    , d(new ConsumerPrivate())
{
    connect(d->cache.get(), &ActivitiesCache::currentActivityChanged, this, &Consumer::currentActivityChanged);
    connect(d->cache.get(), &ActivitiesCache::activityAdded, this, &Consumer::activityAdded);
    connect(d->cache.get(), &ActivitiesCache::activityRemoved, this, &Consumer::activityRemoved);
    connect(d->cache.get(), &ActivitiesCache::serviceStatusChanged, this, &Consumer::serviceStatusChanged);

    connect(d->cache.get(), &ActivitiesCache::activityListChanged, this, [=]() {
        Q_EMIT activitiesChanged(activities());
    });
    connect(d->cache.get(), &ActivitiesCache::runningActivityListChanged, this, [=]() {
        Q_EMIT runningActivitiesChanged(runningActivities());
    });

    // connect(d->cache.get(), SIGNAL(activityStateChanged(QString,int)),
    //         this, SIGNAL(activityStateChanged(QString,int)));
}

Consumer::~Consumer()
{
    qCDebug(KAMD_CORELIB) << "Killing the consumer";
}

QString Consumer::currentActivity() const
{
    return d->cache->m_currentActivity;
}

QStringList Consumer::activities(Info::State state) const
{
    QStringList result;

    result.reserve(d->cache->m_activities.size());

    for (const auto &info : qAsConst(d->cache->m_activities)) {
        if (info.state == state) {
            result << info.id;
        }
    }

    return result;
}

QStringList Consumer::activities() const
{
    QStringList result;

    result.reserve(d->cache->m_activities.size());

    for (const auto &info : qAsConst(d->cache->m_activities)) {
        result << info.id;
    }

    return result;
}

QStringList Consumer::runningActivities() const
{
    QStringList result;

    result.reserve(d->cache->m_activities.size());

    for (const auto &info : qAsConst(d->cache->m_activities)) {
        if (info.state == Info::Running || info.state == Info::Stopping) {
            result << info.id;
        }
    }

    return result;
}

Consumer::ServiceStatus Consumer::serviceStatus()
{
    return d->cache->m_status;
}

} // namespace KActivities
