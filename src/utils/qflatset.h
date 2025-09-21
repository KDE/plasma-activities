/*
    SPDX-FileCopyrightText: 2016 Ivan Čukić <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KACTIVITIES_STATS_QFLATSET_H
#define KACTIVITIES_STATS_QFLATSET_H

#include <QList>
#include <QPair>

namespace KActivities
{
template<typename T, typename LessThan>
class QFlatSet : public QList<T>
{
public:
    QFlatSet()
    {
    }

    inline std::tuple<typename QList<T>::iterator, int, bool> insert(const T &value)
    {
        auto lessThan = LessThan();

        if (this->begin() == this->end()) {
            QList<T>::insert(0, value);

            return std::make_tuple(QList<T>::begin(), 0, true);

        } else {
            auto iterator = std::lower_bound(this->begin(), this->end(), value, lessThan);

            if (iterator != this->end()) {
                if (!lessThan(value, *iterator)) {
                    // Already present
                    return std::make_tuple(iterator, iterator - this->begin(), false);
                }
            }

            auto newIterator = QList<T>::insert(iterator, value);

            return std::make_tuple(newIterator, newIterator - this->begin(), true);
        }
    }

private:
    QFlatSet(const QFlatSet &original); // = delete
};

} // namespace KActivities

#endif // KACTIVITIES_STATS_QFLATSET_H
