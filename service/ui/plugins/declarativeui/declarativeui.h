/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UI_PLUGINS_DECLARATIVEUI_UI_DECLARATIVE_UI_H
#define UI_PLUGINS_DECLARATIVEUI_UI_DECLARATIVE_UI_H

#include <QObject>

#include "../../UiHandler.h"

class DeclarativeUiHandler: public UiHandler {
    Q_OBJECT

public:
    explicit DeclarativeUiHandler(QObject *parent = 0, const QVariantList & args = QVariantList());
    ~DeclarativeUiHandler();

    virtual void message(const QString & title, const QString & message);
    virtual void askPassword(const QString & title, const QString & message,
            bool newPassword, bool unlockMode, QObject * receiver, const char * slot);
    virtual void ask(const QString & title, const QString & message,
            const QStringList & choices, QObject * receiver, const char * slot);
    virtual void setBusy(bool value = true);

private:
    class Private;
    Private * const d;
};

#endif // UI_PLUGINS_DECLARATIVEUI_UI_DECLARATIVE_UI_H