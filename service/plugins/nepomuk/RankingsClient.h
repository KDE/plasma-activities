/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef RANKINGSCLIENT_H_
#define RANKINGSCLIENT_H_

#include <QObject>
#include <QVariantList>

class RankingsClientPrivate;

/**
 *
 */
class RankingsClient: public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.RankingsClient")

public:
    RankingsClient();
    ~RankingsClient();

public Q_SLOTS:
    void updated(const QVariantList & data);
    void inserted(int position, const QVariantList & item);
    void removed(int position);
    void changed(int position, const QVariantList & item);

private:
    class RankingsClientPrivate * const d;
};

#endif // RANKINGSCLIENT_H_
