/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2001 Christian Hohnstaedt.
 *
 *  All rights reserved.
 *
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  - Neither the name of the author nor the names of its contributors may be 
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * This program links to software with different licenses from:
 *
 *	http://www.openssl.org which includes cryptographic software
 * 	written by Eric Young (eay@cryptsoft.com)"
 *
 *	http://www.sleepycat.com
 *
 *	http://www.trolltech.com
 * 
 *
 *
 * http://www.hohnstaedt.de/xca
 * email: christian@hohnstaedt.de
 *
 * $Id$ 
 *
 */                           


#include "ReqView.h"
#include "widgets/ReqDetail.h"
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include "widgets/MainWindow.h"
#include "widgets/NewX509.h"
#include "widgets/distname.h"
#include "widgets/clicklabel.h"


ReqView::ReqView(QWidget * parent = 0, const char * name = 0, WFlags f = 0)
	:XcaListView(parent, name, f)
{
	addColumn(tr("Common Name"));
	/*
	connect(keyl, SIGNAL(delKey(pki_key *)), this, SLOT(delKey(pki_key *)));
	connect(keyl, SIGNAL(newKey(pki_key *)), this, SLOT(newKey(pki_key *)));
	*/
}

void ReqView::newItem()
{
	newItem(NULL);
}

void ReqView::newItem(pki_temp *temp)
{
	NewX509 *dlg = new NewX509(this,0,true);
	emit connNewX509(dlg);
	 
	if (temp) {
		dlg->defineTemplate(temp);
	}
	dlg->setRequest();
	if (! dlg->exec()){
		delete dlg;
		return;
	}
	try {
		pki_key *key = dlg->getSelectedKey();
		x509name xn = dlg->getX509name();
		pki_x509req *req = new pki_x509req();
		req->setIntName(dlg->description->text());
		req->createReq(key, xn, dlg->getHashAlgo());
		insert(req);
	}
	catch (errorEx &err) {
		Error(err);
	}
}



void ReqView::showItem(pki_base *item, bool import)
{
	if (!item) return;
	emit init_database();
    try {
        ReqDetail *dlg = new ReqDetail(this,0,true);

	dlg->setReq((pki_x509req *)item);
	connect(dlg->privKey, SIGNAL(doubleClicked(QString)),
		this, SLOT(dlg_showKey(QString)));	
	QString odesc = item->getIntName();
	bool ret = dlg->exec();
	QString ndesc = dlg->descr->text();
	delete dlg;
	if (!ret && import) {
		delete (pki_x509req *)item;
	}
	if (!ret) return;
	
	emit init_database();
	
	if (import) {
		item = insert(item);
	}
	
	if (ndesc != odesc) {
			db->renamePKI(item, ndesc);
	}
    }
    catch (errorEx &err) {
	    Error(err);
    }
}

void ReqView::deleteItem()
{
	deleteItem_default(tr("The Certificate signing request"),
		tr("is going to be deleted"));
}

void ReqView::load()
{
	QStringList filter;
	filter.append("PKCS#10 CSR ( *.pem *.der *.csr )"); 
	filter.append("All Files ( *.* )");
	load_default(filter, tr("Import CSR"));
}

pki_base *ReqView::loadItem(QString fname)
{
	pki_base *req = new pki_x509req(fname);
        return req;
}


void ReqView::writeReq_pem() { store(true); }
void ReqView::writeReq_der() { store(false); }

void ReqView::store(bool pem)
{
	pki_x509req *req;
	try {
		req = (pki_x509req *)getSelected();
	}
	catch (errorEx &err) {
		Error(err);
		return;
	}

	if (!req) return;
	QStringList filt;
	filt.append("PKCS#10 CSR ( *.pem *.der *.csr )"); 
	filt.append("All Files ( *.* )");
	QString s="";
	QFileDialog *dlg = new QFileDialog(this,0,true);
	dlg->setCaption(tr("Export Certificate signing request"));
	dlg->setFilters(filt);
	dlg->setMode( QFileDialog::AnyFile );
	dlg->setSelection( req->getIntName() + ".csr" );
	dlg->setDir(MainWindow::getPath());
	if (dlg->exec()) {
		s = dlg->selectedFile();
		MainWindow::setPath(dlg->dirPath());
	}
	delete dlg;
	if (s.isEmpty()) return;
	s=QDir::convertSeparators(s);
	try {
		req->writeReq(s, pem);
	}
	catch (errorEx &err) {
		Error(err);
	}
}

void ReqView::signReq()
{
	pki_x509req *req;
	try {
		req = (pki_x509req *)getSelected();
	}
	catch (errorEx &err) {
		Error(err);
		return;
	}
	newCert(req);
}


pki_base *ReqView::insert(pki_base *item)
{
	pki_x509req *oldreq, *req;
	req = (pki_x509req *)item;
	emit init_database();
	try {
		oldreq = (pki_x509req *)db->getByReference(req);
	}
	catch (errorEx &err) {
		Error(err);
	}
	if (oldreq) {
	   QMessageBox::information(this,tr(XCA_TITLE),
		tr("The certificate signing request already exists in the database as") +":\n'" +
		oldreq->getIntName() + 
		"'\n" + tr("and thus was not stored"), "OK");
	   delete(req);
	   return oldreq;
	}
	try {
		db->insertPKI(req);
	}
	catch (errorEx &err) {
		Error(err);
	}
	updateView();
	return req;
}


void ReqView::popupMenu(QListViewItem *item, const QPoint &pt, int x) {
	CERR("hallo popup Req");
	QPopupMenu *menu = new QPopupMenu(this);
	QPopupMenu *subExport = new QPopupMenu(this);
	if (!item) {
		menu->insertItem(tr("New Request"), this, SLOT(newItem()));
		menu->insertItem(tr("Import"), this, SLOT(load()));
	}
	else {
		menu->insertItem(tr("Rename"), this, SLOT(startRename()));
		menu->insertItem(tr("Show Details"), this, SLOT(showItem()));
		menu->insertItem(tr("Sign"), this, SLOT(signReq()));
		menu->insertItem(tr("Export"), subExport);
		subExport->insertItem(tr("PEM"), this, SLOT(writeReq_pem()));
		subExport->insertItem(tr("DER"), this, SLOT(writeReq_der()));
		menu->insertItem(tr("Delete"), this, SLOT(deleteItem()));
	}
	menu->exec(pt);
	delete menu;
	delete subExport;
	return;
}
