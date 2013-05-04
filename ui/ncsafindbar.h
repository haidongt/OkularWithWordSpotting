/***************************************************************************
 *   Copyright (C) 2007 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _NCSAFINDBAR_H_
#define _NCSAFINDBAR_H_

#include <qwidget.h>
#include <QLabel>
#include <QComboBox>

#include <core/document.h>
#include <core/observer.h>

#include <leptonica/allheaders.h>

#include "ncsawordspottingutil.h"

class QAction;
class SearchLineWidget;

namespace Okular {
class Document;
}

class NCSAFindBar
    : public QWidget, private Okular::DocumentObserver
{
    Q_OBJECT

    public:
        explicit NCSAFindBar( Okular::Document * document, QWidget * parent = 0 );
        virtual ~NCSAFindBar();

        QString text() const;
        Qt::CaseSensitivity caseSensitivity() const;

        void focusAndSetCursor();
        bool maybeHide();
	
	
	enum {OKULAR_OBSERVER_ID = 6};
        uint observerId() const {
            return OKULAR_OBSERVER_ID;
        }
        void notifyPageChanged(int page, int flags);

    signals:
        void searchResultSelected(int pageNum, double x, double y, double w, double h);

    //public slots:
        //void findNext();
        //void findPrev();
        //void resetSearch();

    private slots:
        //void caseSensitivityChanged();
        //void fromCurrentPageChanged();
        //void closeAndStopSearch();
	void changeFont(int index);
	void searchLineTextChanged(QString text);
	void performSearch();

    private:
        Okular::Document * doc;
        //SearchLineWidget * m_search;
	QLineEdit *searchLine;
	QLabel * display;
	QComboBox *fontComboBox;
	QComboBox *resultComboBox;
        //QAction * m_caseSensitiveAct;
        //QAction * m_fromCurrentPageAct;
        //bool eventFilter( QObject *target, QEvent *event );
        bool m_active;
	void saveTempPics();
	
	void sendRequest(const Okular::Page *page, int width, int height);
	QString getFilePath();
	
};


#endif
