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

/**
 * Data model that shows existing activities
 */
class PLASMA_ACTIVITIES_EXPORT ActivitiesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ActivitiesModel(QObject *parent = nullptr);

    ~ActivitiesModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    enum Roles {
        ActivityId = Qt::UserRole, ///< UUID of the activity
        ActivityName = Qt::UserRole + 1, ///< Activity name
        ActivityDescription = Qt::UserRole + 2, ///< Activity description
        ActivityIconSource = Qt::UserRole + 3, ///< Activity icon source name
        ActivityBackground = Qt::UserRole + 5, ///< Activity wallpaper (currently unsupported)
        ActivityIsCurrent = Qt::UserRole + 6, ///< Is this activity the current one current

        UserRole = Qt::UserRole + 32, ///< To be used by models that inherit this one
    };

private:
    friend class ActivitiesModelPrivate;
    const std::unique_ptr<ActivitiesModelPrivate> d;
};

} // namespace KActivities

#endif // ACTIVITIES_ACTIVITIESMODEL_H
