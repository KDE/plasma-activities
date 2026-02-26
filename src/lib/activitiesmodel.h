/*
    SPDX-FileCopyrightText: 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIVITIES_ACTIVITIESMODEL_H
#define ACTIVITIES_ACTIVITIESMODEL_H

// Qt
#include <QAbstractListModel>
#include <QObject>

// STL
#include <memory>

// Local
#include "info.h"

class QModelIndex;
class QDBusPendingCallWatcher;

namespace KActivities
{
class ActivitiesModelPrivate;

/*!
 * \class KActivities::ActivitiesModel
 * \inmodule PlasmaActivities
 * \inheaderfile PlasmaActivities/ActivitiesModel
 *
 * \brief Data model that shows existing activities.
 */
class PLASMA_ACTIVITIES_EXPORT ActivitiesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /*!
     *
     */
    explicit ActivitiesModel(QObject *parent = nullptr);

    ~ActivitiesModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    /*!
     * \value ActivityId UUID of the activity
     * \value ActivityName Activity name
     * \value ActivityDescription Activity description
     * \value ActivityIconSource Activity icon source name
     * \value ActivityBackground Activity wallpaper (currently unsupported)
     * \value ActivityIsCurrent Is this activity the current one current
     * \value UserRole To be used by models that inherit this one
     */
    enum Roles {
        ActivityId = Qt::UserRole,
        ActivityName = Qt::UserRole + 1,
        ActivityDescription = Qt::UserRole + 2,
        ActivityIconSource = Qt::UserRole + 3,
        ActivityBackground = Qt::UserRole + 5,
        ActivityIsCurrent = Qt::UserRole + 6,

        UserRole = Qt::UserRole + 32,
    };

private:
    friend class ActivitiesModelPrivate;
    const std::unique_ptr<ActivitiesModelPrivate> d;
};

} // namespace KActivities

#endif // ACTIVITIES_ACTIVITIESMODEL_H
