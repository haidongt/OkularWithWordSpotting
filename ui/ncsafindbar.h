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
#include <QDialog>
#include <qlayout.h>

#include <core/document.h>
#include <core/observer.h>

#include <leptonica/allheaders.h>

#include "ncsawordspottingutil.h"
#include <poppler/qt4/poppler-qt4.h>

class QAction;
class SearchLineWidget;

namespace Okular {
class Document;
}


#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QScrollArea>
#include <QButtonGroup>
#include <map>
#include <vector>
#include <QLabel>
#include <QRadioButton>

class ImageLabel : public QLabel
{
    Q_OBJECT
protected:
  void mousePressEvent(QMouseEvent * e);
signals:
  void clicked(const QPoint & pos);
};

class BuildFontDialog:public QDialog
{
  Q_OBJECT

public:
    BuildFontDialog(Poppler::Document *pdf);

public slots:
    void goPrevious();
    void goNext();
    void testFun();
    void displayOriginalPage();
    void displayBoxedPage();
    void pickLetter(const QPoint & p);


private:

    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
    QScrollArea * scrollArea;
    ImageLabel * imageLabel;
    QLabel * pageIndicator;
    QImage originalPage;
    QImage boxedPage;

    Poppler::Document *doc;
    int currentPage;
    
    void displayPage();
    void createLetters(QHBoxLayout *lowerCases, QHBoxLayout *upperCases);
    QButtonGroup *buttonGroup;
    QButtonGroup isBoxedDisplay;
    std::map<QAbstractButton*, QLabel*> radio2label;
    std::vector<QRect> letterRects;

};




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
	void resultComboBoxIndexChanged(int index);
	void buildFontAct(bool b);

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
	
	QString getFilePath();
	vector<NCSAWordInfo*> searchResult;
	NCSAWordSpottingUtil* wordSpottingUtil;
	
	void preprocessDocument();
	QAction *m_buildFontAct;

	
};


#endif
