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
#include <QFileDialog>

#include <core/generator.h>
#include <tesseract/baseapi.h>

#include <kmessagebox.h>

void ImageLabel::mousePressEvent(QMouseEvent * e)
{
  QLabel::mousePressEvent(e);
  emit clicked(e->pos());
}

void ImageLabel::dragEnterEvent(QDragEnterEvent * e)
{
  QLabel::dragEnterEvent(e);
  qDebug() << "enter drag";
}
void ImageLabel::dragMoveEvent(QDragMoveEvent * e)
{
  QLabel::dragMoveEvent(e);
  qDebug() << "move drag";
}
void ImageLabel::dragLeaveEvent(QDragLeaveEvent * e)
{
  QLabel::dragLeaveEvent(e);
  qDebug() << "leave drag";
}

    
BuildFontDialog::BuildFontDialog(Poppler::Document *pdf)
{

    doc = pdf;
    currentPage = 0;
    QPushButton *saveToFileButton = new QPushButton(tr("Save"));
    QPushButton *confirmButton = new QPushButton(tr("OK"));
    QPushButton *closeButton = new QPushButton(tr("Cancel"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(confirmButton, SIGNAL(clicked()), this, SLOT(testFun()));
    connect(saveToFileButton, SIGNAL(clicked()), this, SLOT(saveToFile()));

    imageLabel = new ImageLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    connect(imageLabel, SIGNAL(clicked(const QPoint & )), this, SLOT(pickLetter(const QPoint & )));
    
    QPushButton *previousButton = new QPushButton(tr("Previous"));
    QPushButton *nextButton = new QPushButton(tr("Next"));
    connect(previousButton, SIGNAL(clicked()), this, SLOT(goPrevious()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(goNext()));
    pageIndicator = new QLabel;
    QRadioButton *displayOriginal = new QRadioButton(tr("Original"));
    QRadioButton *displayBoxed = new QRadioButton(tr("Boxed"));
    isBoxedDisplay.addButton(displayOriginal);
    isBoxedDisplay.addButton(displayBoxed);
    displayOriginal->setChecked(true);
    
    selectionArbitrary = new QCheckBox();
    selectionArbitrary->setChecked(false);
    selectionArbitrary->setText("Arbitrary Rectangle");
    arbitraryLastPoint.setX(-1);
    
    connect(displayOriginal, SIGNAL(clicked()), this, SLOT(displayOriginalPage()));
    connect(displayBoxed, SIGNAL(clicked()), this, SLOT(displayBoxedPage()));

    QHBoxLayout *pageNavigationLayout = new QHBoxLayout;
    pageNavigationLayout->addStretch();
    pageNavigationLayout->addWidget(previousButton);
    pageNavigationLayout->addWidget(pageIndicator);
    pageNavigationLayout->addWidget(nextButton);
    pageNavigationLayout->addStretch();
    pageNavigationLayout->addWidget(selectionArbitrary);
    pageNavigationLayout->addStretch();
    pageNavigationLayout->addWidget(displayOriginal);
    pageNavigationLayout->addWidget(displayBoxed);
    pageNavigationLayout->addStretch();
    
    
    QHBoxLayout *lowerCases = new QHBoxLayout;
    QHBoxLayout *upperCases = new QHBoxLayout;
    buttonGroup= new QButtonGroup;
    createLetters(lowerCases, upperCases);
    connect(buttonGroup,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(displayFontEditorFor(QAbstractButton*)));

    QVBoxLayout *lettersLayout = new QVBoxLayout;
    lettersLayout->addLayout(lowerCases);
    lettersLayout->addLayout(upperCases);
    
    QHBoxLayout *letterAdjustAndLetters = new QHBoxLayout;
    fontEditor = new NCSAFontEditor(&builtFont);
    letterAdjustAndLetters->addLayout(fontEditor);
    letterAdjustAndLetters->addLayout(lettersLayout);

    QHBoxLayout *testLayout = new QHBoxLayout;
    testInput = new QLineEdit;
    testDisplay = new QLabel;
    testLayout->addWidget(testInput);
    testLayout->addStretch();
    testLayout->addWidget(testDisplay);
    connect(testInput,SIGNAL(textChanged(QString)),this,SLOT(testInputChanged(QString)));
    
    QVBoxLayout *verticalLayout = new QVBoxLayout;
    verticalLayout->addWidget(scrollArea);
    verticalLayout->addLayout(pageNavigationLayout);
    verticalLayout->addLayout(letterAdjustAndLetters);
    verticalLayout->addLayout(testLayout);


    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(saveToFileButton);
    buttonsLayout->addWidget(confirmButton);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(verticalLayout);
    //mainLayout->addStretch(1);
    //mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
    resize(1000, 600);
   
    setWindowTitle(tr("Font Creater"));
    
    displayPage();
}



void BuildFontDialog::testFun()
{
  QLabel* label = radio2label[this->buttonGroup->checkedButton()];
  
  label->setText("X");
  
  QRadioButton *next= (QRadioButton *)(this->buttonGroup->button(this->buttonGroup->id(this->buttonGroup->checkedButton())-1));
  if(next != NULL)
  {
    next->setChecked(true);
  }
}

void BuildFontDialog::createLetters(QHBoxLayout *lowerCases, QHBoxLayout *upperCases)
{

      vector<char*> lowerLetterList;
      vector<char*> upperLetterList;

      lowerLetterList.push_back("a");
      lowerLetterList.push_back("b");
      lowerLetterList.push_back("c");
      lowerLetterList.push_back("d");
      lowerLetterList.push_back("e");
      lowerLetterList.push_back("f");
      lowerLetterList.push_back("g");
      lowerLetterList.push_back("h");
      lowerLetterList.push_back("i");
      lowerLetterList.push_back("j");
      lowerLetterList.push_back("k");
      lowerLetterList.push_back("l");
      lowerLetterList.push_back("m");
      lowerLetterList.push_back("n");
      lowerLetterList.push_back("o");
      lowerLetterList.push_back("p");
      lowerLetterList.push_back("q");
      lowerLetterList.push_back("r");
      lowerLetterList.push_back("s");
      lowerLetterList.push_back("t");
      lowerLetterList.push_back("u");
      lowerLetterList.push_back("v");
      lowerLetterList.push_back("w");
      lowerLetterList.push_back("x");
      lowerLetterList.push_back("y");
      lowerLetterList.push_back("z");
      upperLetterList.push_back("A");
      upperLetterList.push_back("B");
      upperLetterList.push_back("C");
      upperLetterList.push_back("D");
      upperLetterList.push_back("E");
      upperLetterList.push_back("F");
      upperLetterList.push_back("G");
      upperLetterList.push_back("H");
      upperLetterList.push_back("I");
      upperLetterList.push_back("J");
      upperLetterList.push_back("K");
      upperLetterList.push_back("L");
      upperLetterList.push_back("M");
      upperLetterList.push_back("N");
      upperLetterList.push_back("O");
      upperLetterList.push_back("P");
      upperLetterList.push_back("Q");
      upperLetterList.push_back("R");
      upperLetterList.push_back("S");
      upperLetterList.push_back("T");
      upperLetterList.push_back("U");
      upperLetterList.push_back("V");
      upperLetterList.push_back("W");
      upperLetterList.push_back("X");
      upperLetterList.push_back("Y");
      upperLetterList.push_back("Z");

      
      lowerCases->addStretch();
      for(int i = 0; i < lowerLetterList.size(); i++)
      {

	QVBoxLayout *layout = new QVBoxLayout();
	QLabel *letterImg = new QLabel();
	QRadioButton *a = new QRadioButton(lowerLetterList[i]);
	radioBtn2Char[a] = lowerLetterList[i][0];
	radio2label[a] = letterImg;
        layout->addWidget(letterImg);
	layout->addWidget(a);
	lowerCases->addLayout(layout);
	this->buttonGroup->addButton(a);
	if(i == 0) a->setChecked(true);
      }
      lowerCases->addStretch();
      
      upperCases->addStretch();
      for(int i = 0; i < upperLetterList.size(); i++)
      {
	QVBoxLayout *layout = new QVBoxLayout();
	QLabel *letterImg = new QLabel();
	QRadioButton *a = new QRadioButton(upperLetterList[i]);
	radioBtn2Char[a] = upperLetterList[i][0];
	radio2label[a] = letterImg;
        layout->addWidget(letterImg);
	layout->addWidget(a);
	upperCases->addLayout(layout);
	this->buttonGroup->addButton(a);
      }
      upperCases->addStretch();
  
}


void BuildFontDialog::goPrevious()
{
  currentPage = currentPage - 1;
  if(currentPage < 0)
  {
    currentPage = 0;
  }
  displayPage();
}

void BuildFontDialog::goNext()
{
  
  currentPage = currentPage + 1;
  if(currentPage >= doc->numPages())
  {
    currentPage = doc->numPages()-1;
  }
  displayPage();

}

void BuildFontDialog::displayOriginalPage()
{
  imageLabel->setPixmap(QPixmap::fromImage(originalPage));
  imageLabel->adjustSize();
}
void BuildFontDialog::displayBoxedPage()
{
  imageLabel->setPixmap(QPixmap::fromImage(boxedPage));
  imageLabel->adjustSize();
}
void BuildFontDialog::displayPage()
{
  originalPage = doc->page(currentPage)->renderToImage(150, 150);
  PIX* pix  = NCSAWordSpottingUtil::qImage2PIX(originalPage);
  
  tesseract::TessBaseAPI tess;
    tess.Init(NULL, "eng");
    
    tess.SetImage(pix);

    tess.Recognize(NULL);
    tesseract::ResultIterator* ri = tess.GetIterator();
    tesseract::ChoiceIterator* ci;
    
    boxedPage = originalPage.copy();
    QPainter p;
    p.begin(&boxedPage);
    p.setPen(Qt::red);
    letterRects.clear();
    
    QRect lastLetterBoxTight(0,0,0,0);
    QRect lastWordBox(0,0,0,0);

    if(ri != NULL)
    {
      do
      {
	int left, top, right, bottom;
	ri->BoundingBox(tesseract::RIL_SYMBOL, &left, &top, &right, &bottom);
	int x1, y1, x2, y2;
	ri->Baseline(tesseract::RIL_SYMBOL, &x1, &y1, &x2, &y2);
	int baseline = y1 + ((left+right)/2-x1)*(y2-y1)/(x2-x1);
	baseline = baseline - top;
	QRect letterBoxTight = QRect(left, top, right-left, bottom-top);
	QRect letterBox = letterBoxTight;
	QRect wordBox = letterBoxTight;
	//tightLetterBox2Box[letterBoxTight] = letterBox;
	
	wordBox.setLeft(x1);
	wordBox.setRight(x2);
	letterRects.push_back(letterBoxTight);
	baselines.push_back(baseline);
	p.drawRect(letterBoxTight);
	
	/*
	if (lastLetterBoxTight.x() <= wordBox.x())
	{
	  QRect lastLetterBox = tightLetterBox2Box[lastLetterBoxTight];
	  int X1 = lastLetterBoxTight.x()+lastLetterBoxTight.width();
	  int X2 = letterBoxTight.x();
	  tightLetterBox2Box[lastLetterBoxTight] = QRect(lastLetterBox.x(),lastLetterBox.top(), (X1+X2)/2 - lastLetterBox.x(), lastLetterBox.height());
	  tightLetterBox2Box[letterBoxTight] = QRect((X1+X2)/2,letterBox.top(), letterBox.width(), letterBox.height());

	}*/
	
	lastLetterBoxTight = letterBoxTight;
	lastWordBox = wordBox;

      }
      while(ri->Next(tesseract::RIL_SYMBOL));
    }
    p.end();
      
  
  imageLabel->setPixmap(QPixmap::fromImage(originalPage));
  imageLabel->adjustSize();
  QString pageText;
  pageText.append(QString::number(currentPage+1));
  pageText.append("/");
  pageText.append(QString::number(doc->numPages()));
  pageIndicator->setText(pageText);
  
  isBoxedDisplay.checkedButton()->click();


}

void BuildFontDialog::pickLetter(const QPoint & p)
{ 
  if (selectionArbitrary->isChecked())
  {
    if(arbitraryLastPoint.x() < 0)
    {
      arbitraryLastPoint = p;
    }
    else
    {
      QRect rect(arbitraryLastPoint, p);
      if(rect.height() <= 0) rect.setHeight(10);
      if(rect.width() <= 0) rect.setWidth(10); //TODO Code smell!!
      QImage picked = originalPage.copy(rect);
      QLabel* label = radio2label[this->buttonGroup->checkedButton()]; //TODO duplicate code!!!
      label->setPixmap(QPixmap::fromImage(picked));
      label->adjustSize();
      FontLetterInfo info;
      info.img = QPixmap::fromImage(picked);
      info.topLeft = QPoint(-1,-picked.height()) ;
      info.width = info.img.width();
      char letter =  radioBtn2Char[(QRadioButton*)(this->buttonGroup->checkedButton())];
      builtFont[letter] = info;
      
      QRadioButton *next= (QRadioButton *)(this->buttonGroup->button(this->buttonGroup->id(this->buttonGroup->checkedButton())-1));
      if(next != NULL)
      {
        next->setChecked(true);
      }
      
      arbitraryLastPoint.setX(-1);
    }
    return;
  }
  for(int i = 0; i < letterRects.size(); i++)
  {
    QRect rect = letterRects[i];

    if(p.x() >= rect.left() && p.x() <= rect.right() && p.y() >= rect.top() && p.y() <= rect.bottom())
    {
      //qDebug() << "wrapped in" << p << rect;
      QImage picked = originalPage.copy(rect);
      QLabel* label = radio2label[this->buttonGroup->checkedButton()];
      label->setPixmap(QPixmap::fromImage(picked));
      label->adjustSize();
      
      
      
      int baseline = baselines[i]; //TODO : vulnerable!!!
      char letter =  radioBtn2Char[(QRadioButton*)(this->buttonGroup->checkedButton())];
      FontLetterInfo info;
      info.img = QPixmap::fromImage(picked);
      qDebug() << "baseline" << info.img.height() << baseline;
      info.topLeft = QPoint(-1,-baseline) ;
      info.width = info.img.width();
      builtFont[letter] = info;
      qDebug() << "add to map" << letter;
      
      
      QRadioButton *next= (QRadioButton *)(this->buttonGroup->button(this->buttonGroup->id(this->buttonGroup->checkedButton())-1));
      if(next != NULL)
      {
        next->setChecked(true);
      }
      
      break;
    }
  }
  
}

void BuildFontDialog::testInputChanged(QString str)
{
  std::vector<FontLetterInfo> fontLetters;  
  for(int i = 0; i < str.size(); i++)
  {
    char letter = str.at(i).toAscii();
    if(builtFont.count(letter) != 0)
    {
      fontLetters.push_back(builtFont[letter]);
      qDebug() << letter;
    }
  }  
  
  QPixmap renderedText = renderFont2Pix(fontLetters);
  testDisplay->setPixmap(renderedText);
  testDisplay->adjustSize();


}

QPixmap BuildFontDialog::renderFont2Pix(std::vector<FontLetterInfo> fontLetters)
{
  int accumulatedWidth = 0;
  int leftMost = 0;
  int rightMost = 0;
  int topMost = 0;
  int bottomMost = 0;
  
  for(int i = 0; i < fontLetters.size(); i++)
  {
    if(accumulatedWidth + fontLetters[i].topLeft.x() < leftMost)
    {
      leftMost = accumulatedWidth + fontLetters[i].topLeft.x();
    }
    if( accumulatedWidth + fontLetters[i].topLeft.x() + fontLetters[i].img.width() > rightMost)
    {
      rightMost = accumulatedWidth + fontLetters[i].topLeft.x() + fontLetters[i].img.width();
    }
    if(fontLetters[i].topLeft.y() < topMost)
    {
      topMost = fontLetters[i].topLeft.y();
    }
    if(fontLetters[i].topLeft.y() + fontLetters[i].img.height() > bottomMost)
    {
      bottomMost = fontLetters[i].topLeft.y() + fontLetters[i].img.height();
    }
    accumulatedWidth += fontLetters[i].width;
  }
  
  qDebug() << leftMost << rightMost << topMost << bottomMost;
  QPixmap rendered(rightMost-leftMost, bottomMost - topMost);
  rendered.fill(Qt::transparent);
  QPainter p(&rendered);
  
  int baseline = 0 - topMost;
  int leftMargin = 0 - leftMost;
  
  accumulatedWidth = 0;
  for(int i = 0; i < fontLetters.size(); i++)
  {
    int x = accumulatedWidth + leftMargin + fontLetters[i].topLeft.x();
    qDebug() << "decomposed" << accumulatedWidth << leftMargin << fontLetters[i].topLeft.x();// x << y;
    int y = baseline + fontLetters[i].topLeft.y();
    qDebug() << x << y;
    p.drawPixmap(x, y, fontLetters[i].img);
    accumulatedWidth += fontLetters[i].width;
  }
  


  return rendered;
}


void BuildFontDialog::saveToFile()
{
  
  QString path = QFileDialog::getSaveFileName(this, "Save file", "", ".conf");
  QDir dir = QDir::root();
  if(!dir.mkdir(path))
  {
    qDebug() << "cannot create folder!";
  }
  else
  {
  }
  
}

void BuildFontDialog::displayFontEditorFor(QAbstractButton*)
{
  char letter =  radioBtn2Char[(QRadioButton*)(this->buttonGroup->checkedButton())];
  fontEditor->displayEditorFor(letter);
}





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


    QPushButton * optionsBtn = new QPushButton( this );
    optionsBtn->setText( i18n( "NCSA Options" ) );
    optionsBtn->setToolTip( i18n( "Modify search behavior" ) );
    QMenu * optionsMenu = new QMenu( optionsBtn );
    m_buildFontAct = optionsMenu->addAction( i18n( "Create custom font" ) );
    connect( m_buildFontAct, SIGNAL(triggered(bool)), this, SLOT(buildFontAct(bool)) );

    optionsBtn->setMenu( optionsMenu );
    lay->addWidget( optionsBtn );
    
    QStringList fonts = QFontDatabase().families();
    
    fontComboBox = new QComboBox();
    fontComboBox->addItems(fonts);
    lay->addWidget(fontComboBox);
    connect( fontComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFont(int)) );
    connect( resultComboBox, SIGNAL(highlighted(int)), this, SLOT(resultComboBoxIndexChanged(int)) );

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
    wordSpottingUtil = NULL;
    
}

NCSAFindBar::~NCSAFindBar()
{
}


void NCSAFindBar::buildFontAct(bool b)
{
  QString filepath =  getFilePath();
  Poppler::Document *pdf = Poppler::Document::load(filepath);  
  BuildFontDialog* dialog = new BuildFontDialog(pdf);
  dialog->show();
}


QString NCSAFindBar::getFilePath()
{
  
   const QDomDocument * ds = (const QDomDocument *)(doc->documentInfo());
   if(ds == NULL)
   {
     QMessageBox::warning(0, QString("Error"), QString("Cannot get file path."));
     return "";

   }
   QString filename = ds->toString();
   int begin_idx = filename.indexOf("<filePath title=\"File Path\" value=") ;
   int end_idx = filename.indexOf("/>", begin_idx);
   return filename.mid(begin_idx+42, end_idx - begin_idx-43);
  
}

  
void NCSAFindBar::notifyPageChanged(int page, int flags)
{
  qDebug() << "request callback";
}

void NCSAFindBar::resultComboBoxIndexChanged(int index)
{
       NCSAWordInfo* wordInfo = searchResult[index];
       emit searchResultSelected(wordInfo->pagenum,
				     wordInfo->box->x * 1.0 / wordInfo->width, 
				     wordInfo->box->y * 1.0 / wordInfo->height, 
			             wordInfo->box->w * 1.0 / wordInfo->width, 
			             wordInfo->box->h * 1.0 / wordInfo->height);

}

void NCSAFindBar::preprocessDocument()
{
  
     wordSpottingUtil = new NCSAWordSpottingUtil();
     QString filepath =  getFilePath();
     Poppler::Document *pdf = Poppler::Document::load(filepath);  

     if(pdf == NULL)
     {
        QMessageBox::warning(0, QString("Error"), QString("Cannot locate file at path: ").append(filepath).append(" ."));
        return;
     }
     
     for(int i = 0; i < pdf->numPages(); i++)
     {
       wordSpottingUtil->addPage(pdf->page(i)->renderToImage(150, 150), i);
     }
}


void NCSAFindBar::performSearch()
{
  
  if(wordSpottingUtil == NULL)
  {
    this->preprocessDocument();
  }
     
     QPixmap search_input(display->size());
     display->render(&search_input);     
     
     searchResult = wordSpottingUtil->search(search_input, 10);
     
     
     QString filepath =  getFilePath();
     Poppler::Document *pdf = Poppler::Document::load(filepath);  
     
     ////////displaying results
     resultComboBox->clear();
     for(int i = 0; i < searchResult.size(); i++)
     {
       NCSAWordInfo* wordInfo = searchResult[i];
       QImage returnedPage = pdf->page(wordInfo->pagenum)->renderToImage(150,150); //TODO: get rid of having to read the file again
       //QImage returnedPage = *(wordInfo->page);
       const QImage word = returnedPage.copy(wordInfo->box->x, wordInfo->box->y, wordInfo->box->w, wordInfo->box->h);

       resultComboBox->addItem(QPixmap::fromImage(word), "", -1);
       returnedPage.height();
       returnedPage.width();

     }
     QSize size(100,30);
     resultComboBox->setIconSize(size);
     
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

#include "ncsafindbar.moc"
