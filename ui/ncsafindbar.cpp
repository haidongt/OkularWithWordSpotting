/***************************************************************************
 *   Copyright (C) 2007 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "ncsafindbar.h"

// qt/kde includes
#include <qlabel.h>
#include <qlayout.h>
#include <qmenu.h>
#include <qtoolbutton.h>
#include <qevent.h>
#include <kicon.h>
#include <klocale.h>
#include <kpushbutton.h>

// local includes
#include "searchlineedit.h"
#include "core/document.h"
#include <core/page.h>
#include "settings.h"

#include <QFontDatabase>
#include <qcombobox.h>
#include <QStringList>
#include <QPainter>
#include <QLabel>
#include <core/generator.h>
    
#include <poppler/qt4/poppler-qt4.h>

#include <kmessagebox.h>



NCSAFindBar::NCSAFindBar( Okular::Document * document, QWidget * parent )
  : QWidget( parent )
  , m_active( false )
{
    doc = document;
    if(NULL != doc)
    {
      doc->addObserver(this);
    }
    QVBoxLayout * vlay = new QVBoxLayout( this );
    QHBoxLayout * displayRow = new QHBoxLayout();
    vlay->addLayout( displayRow );
    display = new QLabel("");
    displayRow->addWidget(display);
    displayRow->insertStretch(1);
    resultComboBox = new QComboBox();
    //resultComboBox->addItems(fonts);
    displayRow->addWidget(resultComboBox);

    QHBoxLayout * lay = new QHBoxLayout();
    vlay->setMargin( 2 );
    vlay->addLayout(lay);

    /*
    QToolButton * closeBtn = new QToolButton( this );
    closeBtn->setIcon( KIcon( "dialog-close" ) );
    closeBtn->setToolTip( i18n( "Close" ) );
    closeBtn->setAutoRaise( true );
    lay->addWidget( closeBtn );

    QLabel * label = new QLabel( i18nc( "NCSA Find text", "NCSA F&ind:" ), this );
    //lay->addWidget( label );

    m_search = new SearchLineWidget( this, document );
    m_search->lineEdit()->setSearchCaseSensitivity( Qt::CaseInsensitive );
    m_search->lineEdit()->setSearchMinimumLength( 0 );
    m_search->lineEdit()->setSearchType( Okular::Document::NextMatch );
    m_search->lineEdit()->setSearchId( PART_SEARCH_ID );
    m_search->lineEdit()->setSearchColor( qRgb( 255, 255, 64 ) );
    m_search->lineEdit()->setSearchMoveViewport( true );
    m_search->lineEdit()->setToolTip( i18n( "Text to search for" ) );
    m_search->installEventFilter( this );
    label->setBuddy( m_search->lineEdit() );
    //lay->addWidget( m_search );

    QPushButton * findNextBtn = new QPushButton( KIcon( "go-down-search" ), i18nc( "NCSA Find and go to the next search match", "Next" ), this );
    findNextBtn->setToolTip( i18n( "Jump to next match" ) );
    //lay->addWidget( findNextBtn );

    QPushButton * findPrevBtn = new QPushButton( KIcon( "go-up-search" ), i18nc( "Find and go to the previous search match", "Previous" ), this );
    findPrevBtn->setToolTip( i18n( "Jump to previous match" ) );
    //lay->addWidget( findPrevBtn );
    */

    QPushButton * optionsBtn = new QPushButton( this );
    optionsBtn->setText( i18n( "NCSA Options" ) );
    optionsBtn->setToolTip( i18n( "Modify search behavior" ) );
    QMenu * optionsMenu = new QMenu( optionsBtn );
    //m_caseSensitiveAct = optionsMenu->addAction( i18n( "Case sensitive" ) );
    //m_caseSensitiveAct->setCheckable( true );
    //m_fromCurrentPageAct = optionsMenu->addAction( i18n( "From current page" ) );
    //m_fromCurrentPageAct->setCheckable( true );
    optionsBtn->setMenu( optionsMenu );
    lay->addWidget( optionsBtn );

    /*
    connect( closeBtn, SIGNAL(clicked()), this, SLOT(closeAndStopSearch()) );
    connect( findNextBtn, SIGNAL(clicked()), this, SLOT(findNext()) );
    connect( findPrevBtn, SIGNAL(clicked()), this, SLOT(findPrev()) );
    connect( m_caseSensitiveAct, SIGNAL(toggled(bool)), this, SLOT(caseSensitivityChanged()) );
    connect( m_fromCurrentPageAct, SIGNAL(toggled(bool)), this, SLOT(fromCurrentPageChanged()) );
*/
    //m_caseSensitiveAct->setChecked( Okular::Settings::searchCaseSensitive() );
    //m_fromCurrentPageAct->setChecked( Okular::Settings::searchFromCurrentPage() );
    
    
    QStringList fonts = QFontDatabase().families();
    
    fontComboBox = new QComboBox();
    fontComboBox->addItems(fonts);
    lay->addWidget(fontComboBox);
    connect( fontComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFont(int)) );

    QPalette p(palette());
    p.setColor(QPalette::Background, Qt::white);
    display->setAutoFillBackground(true);
    display->setPalette(p);
    display->setText("Lalala");
    QFont font;
    font.setPointSize(32);
    font.setBold(true);
    font.setFamily(fonts.at(16));

    display->setFont(font);
    
    searchLine = new QLineEdit();
    lay->addWidget(searchLine);
    
    connect(searchLine,SIGNAL(textChanged(QString)),this,SLOT(searchLineTextChanged(QString)));
    hide();
    
    QPushButton * searchBtn = new QPushButton( KIcon( "Search" ), i18nc( "Search using word spotting technique", "search" ), this );
    searchBtn->setToolTip( i18n( "Search using word spotting technique" ) );
    lay->addWidget( searchBtn );
    connect( searchBtn, SIGNAL(clicked()), this, SLOT(performSearch()) );

    // "activate" it only at th very end
    m_active = true;
    
}

NCSAFindBar::~NCSAFindBar()
{
}

/*
void NCSAFindBar::preProcessPage(int &width, int &height, const Okular::Page *page)
{
  qreal factor = 1;
  
  if (mimeType_->is("application/vnd.ms-htmlhelp") ||
    mimeType_->is("application/x-chm")) {
    //for chm docs request page with the true size (factor = 1)
    factor = 1.0;
  } else if (Window::FIT_WIDTH_ZOOM_FACTOR == zoomFactor_) {
    //adjust scale factor to occupy the entire window width
    factor = qreal(winWidth_)/page->width();
  } else {
    factor = zoomFactor_;
  }
  
  width = int(factor*(page->width()));
  height = int(factor*(page->height()));
  //adjust size in order to stay below this threshold (used by okular core library)
  const qint64 area = qint64(width)*qint64(height);
  if(area > 20000000L) {
    qDebug() << "adjust width and height in order to stay below threshold";
    qreal factor2 = qreal(20000000L)/qreal(area);
    height = int(height*factor2);
  }
}
*/


QString NCSAFindBar::getFilePath()
{
  
   const QDomDocument * ds = (const QDomDocument *)(doc->documentInfo());
   if(ds == NULL)
   {
     QMessageBox::warning(0, QString("Error"), QString("Cannot get file path."));

   }
   QString filename = ds->toString();
   int begin_idx = filename.indexOf("<filePath title=\"File Path\" value=") ;
   int end_idx = filename.indexOf("/>", begin_idx);
   return filename.mid(begin_idx+42, end_idx - begin_idx-43);
  
}


//send request for page pixmap
void NCSAFindBar::sendRequest(const Okular::Page *page, int width, int height)
{

   QString filepath =  getFilePath();
   Poppler::Document *pdf = Poppler::Document::load(filepath);  

   if(pdf == NULL)
   {
      QMessageBox::warning(0, QString("Error"), QString("Cannot locate file at path: ").append(filepath).append(" ."));
      return;
   }
   
   Poppler::Page *page1 = pdf->page(0);
   QImage page_img = page1->renderToImage();
   page_img.save("/home/htang14/test_img.jpg");
   
   
   Poppler::Page *page2 = pdf->page(10);
   QImage page_img2 = page2->renderToImage(150,150);
   page_img2.save("/home/htang14/test_img1.jpg");
      
   qDebug() << "has pixmap:" << page->hasPixmap(this);
  
   
  
   if(false == page->hasPixmap(this)) {
       qDebug() << "No Images yet, sending request";
       Okular::PixmapRequest *pixmapRequest = new Okular::PixmapRequest(this, page->number(), width, height, 0, 0);//asynchronous request (the request is deleted by okular core library)

       QLinkedList<Okular::PixmapRequest*> req_;
       req_.clear();
       req_.push_back(pixmapRequest);
       doc->requestPixmaps(req_);
   }
   
   else
   {
     /*
     qDebug() << "Images ready";
     
     qDebug() << "Images ready";
     const QPixmap *pixmap = NULL;
     pixmap = page->_o_nearestPixmap(NCSAFindBar::OKULAR_OBSERVER_ID, -1, -1);

     NCSAWordSpottingUtil aaa;
     aaa.addPage(*pixmap,0);
     
     QPixmap search_input(display->size());
     display->render(&search_input);
     search_input.save("/home/htang14/Desktop/okularoutput/output.png");
     
     
     vector< vector<double> > result = aaa.search(search_input);
     
     resultComboBox->clear();
     for(int i = 0; i < result.size(); i++)
     {
       vector<double> rectangle = result[i];
       const QPixmap word = pixmap->copy(rectangle[0], rectangle[1],rectangle[2],rectangle[3]);
       resultComboBox->addItem(word, "", -1);
     }
     
     
     QSize size(100,30);
     resultComboBox->setIconSize(size);
     */
     
   }
    
    //qDebug() << "PagePainter::sendRequest";
    //if(false == page->hasPixmap(99)) {
    //  qDebug() << "making pixmap request";
    //Okular::PixmapRequest *pixmapRequest = new Okular::PixmapRequest(OkularDocument::OKULAR_OBSERVER_ID, page->number(), width, height, 0, true);//asynchronous request (the request is deleted by okular core library)
    //  req_.clear();
    //  req_.push_back(pixmapRequest);
    //  doc_->requestPixmaps(req_);
    //}
    
    
}
  
void NCSAFindBar::notifyPageChanged(int page, int flags)
{
  qDebug() << "request callback";
}

void NCSAFindBar::performSearch()
{
  
  
  qDebug() << "performing search";
  const QPixmap *pix = NULL;
  if (NULL != doc) {
      const Okular::Page *p = doc->page(0);
      
      if (NULL != p) {
	int width = p->width();
        int height = p->height();
	
	this->sendRequest(p, width, height);
	
	/*
	pix = p->_o_nearestPixmap(6,-1,-1);
	if(pix != NULL)
	{
	}
	else
	{
	    qDebug() << "null pix";

	}
        //pix = p->_o_nearestPixmap(6, -1, -1);
	//p->_o
	
	*/
      }
      else
      {
	qDebug() << "null page";
      }
    }
    else
    {
      qDebug() << "null doc";
    }
    
    
  /*
   * 
       qDebug() << "PagePainter::getPagePixmap";
    const QPixmap *pix = NULL;
    if (NULL != doc_) {
      const Okular::Page *p = doc_->page(page);
      if (NULL != p) {
        pix = p->_o_nearestPixmap(OkularDocument::OKULAR_OBSERVER_ID, -1, -1);
      }
    }
    return pix;
    */
  
}
void NCSAFindBar::searchLineTextChanged(QString text)
{
  display->setText(text);
}

void NCSAFindBar::saveTempPics()
{
    
}

void NCSAFindBar::changeFont(int index)
{
    QFont newfont;
    newfont.setPointSize(32);
    newfont.setBold(true);
    newfont.setFamily(fontComboBox->itemText(index));
    display->setFont(newfont);
}
/*
bool NCSAFindBar::eventFilter( QObject *target, QEvent *event ) {
    if ( target == m_search )
    {
        if ( event->type() == QEvent::KeyPress )
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
            if ( keyEvent->key() == Qt::Key_PageUp || keyEvent->key() == Qt::Key_PageDown ) 
            {
                emit forwardKeyPressEvent( keyEvent );
                return true;
            }
        }
    }
    return false;
}

QString NCSAFindBar::text() const
{
    return m_search->lineEdit()->text();
}

Qt::CaseSensitivity NCSAFindBar::caseSensitivity() const
{
    return m_caseSensitiveAct->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
}

void NCSAFindBar::focusAndSetCursor()
{
    setFocus();
    m_search->lineEdit()->selectAll();
    m_search->lineEdit()->setFocus();
}

bool NCSAFindBar::maybeHide()
{
    if ( m_search->lineEdit()->isSearchRunning() )
    {
        m_search->lineEdit()->stopSearch();
        return false;
    }
    else
    {
        hide();
        return true;
    }
}
*/

/*
void NCSAFindBar::findNext()
{
  display->setText("sdfds");
    //m_search->lineEdit()->setSearchType( Okular::Document::NextMatch );
    //m_search->lineEdit()->findNext();
}

void NCSAFindBar::findPrev()
{
    m_search->lineEdit()->setSearchType( Okular::Document::PreviousMatch );
    m_search->lineEdit()->findPrev();
}

void NCSAFindBar::resetSearch()
{
    m_search->lineEdit()->resetSearch();
}

void NCSAFindBar::caseSensitivityChanged()
{
    m_search->lineEdit()->setSearchCaseSensitivity( m_caseSensitiveAct->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive );
    if ( !m_active )
        return;
    Okular::Settings::setSearchCaseSensitive( m_caseSensitiveAct->isChecked() );
    Okular::Settings::self()->writeConfig();
    m_search->lineEdit()->restartSearch();
}

void NCSAFindBar::fromCurrentPageChanged()
{
    m_search->lineEdit()->setSearchFromStart( !m_fromCurrentPageAct->isChecked() );
    if ( !m_active )
        return;
    Okular::Settings::setSearchFromCurrentPage( m_fromCurrentPageAct->isChecked() );
    Okular::Settings::self()->writeConfig();
}

void NCSAFindBar::closeAndStopSearch()
{
    if ( m_search->lineEdit()->isSearchRunning() )
    {
        m_search->lineEdit()->stopSearch();
    }
    close();
}
*/

#include "ncsafindbar.moc"
