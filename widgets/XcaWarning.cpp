/* vi: set sw=4 ts=4:
 *
 * Copyright (C) 2018 Christian Hohnstaedt.
 *
 * All rights reserved.
 */

#include "XcaWarning.h"
#include "lib/func.h"
#include <QApplication>
#include <QClipboard>
#include <QPushButton>
#include <QDebug>
#include <QSqlDatabase>

xcaWarning::xcaWarning(QWidget *w, const QString &txt,
				QMessageBox::Icon icn)
	: QMessageBox(icn, XCA_TITLE, txt, QMessageBox::NoButton, w)
{
	setTextFormat(Qt::PlainText);
}

void xcaWarning::addButton(QMessageBox::StandardButton button,
				const QString &text)
{
	QPushButton *b = QMessageBox::addButton(button);
	if (b && !text.isEmpty())
		b->setText(text);
}

int xcaWarningGui::showBox(const QString &txt, QMessageBox::Icon icn,
			QMessageBox::StandardButtons b)
{
	xcaWarning *w = new xcaWarning(NULL, txt, icn);
	w->setStandardButtons(b);
	int n = w->exec();
	delete w;
	return n;
}

void xcaWarningGui::information(const QString &msg)
{
	showBox(msg, QMessageBox::Information, QMessageBox::Ok);
}

void xcaWarningGui::warning(const QString &msg)
{
	showBox(msg, QMessageBox::Warning, QMessageBox::Ok);
}

bool xcaWarningGui::yesno(const QString &msg)
{
	return showBox(msg, QMessageBox::Question,
		QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
}

bool xcaWarningGui::okcancel(const QString &msg)
{
	return showBox(msg, QMessageBox::Warning,
		QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok;
}

void xcaWarningGui::error(const errorEx &err)
{
	QString msg = tr("The following error occurred:") +
			"\n" + err.getString();
	xcaWarning box(NULL, msg);
	box.addButton(QMessageBox::Apply, tr("Copy to Clipboard"));
	box.addButton(QMessageBox::Ok);
	if (box.exec() == QMessageBox::Apply) {
		QClipboard *cb = QApplication::clipboard();
		cb->setText(msg);
		if (cb->supportsSelection())
			cb->setText(msg, QClipboard::Selection);
	}
}
