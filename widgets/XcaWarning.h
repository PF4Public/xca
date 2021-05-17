/* vi: set sw=4 ts=4:
 *
 * Copyright (C) 2018 - 2020 Christian Hohnstaedt.
 *
 * All rights reserved.
 */

#ifndef __XCAWARNING_H
#define __XCAWARNING_H

#include "lib/XcaWarningCore.h"
#include <QMessageBox>

class xcaWarning: public QMessageBox
{
    public:
	xcaWarning(QWidget *w, const QString &txt,
			QMessageBox::Icon icn = QMessageBox::Warning);
	void addButton(QMessageBox::StandardButton button,
			const QString &text = QString());
};

class xcaWarningGui : public QObject, public xcaWarning_i
{
    public:
	int showBox(const QString &txt, QMessageBox::Icon icn,
			QMessageBox::StandardButtons b);
	void information(const QString &msg);
	void warning(const QString &msg);
	bool yesno(const QString &msg);
	bool okcancel(const QString &msg);
	void sqlerror(QSqlError err);
	void error(const errorEx &err);
};
#endif
