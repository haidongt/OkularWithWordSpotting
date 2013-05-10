#include "ncsafonteditor.h"
#include <qdebug.h>
#include <QPainter>
#include <QMouseEvent>

NCSAFontEditorDisplay::NCSAFontEditorDisplay(QButtonGroup *bg)
{
  info = NULL;
  buttonGroup =bg;
  connect(this, SIGNAL(clicked(const QPoint & )), this, SLOT(adjustLine(const QPoint & )));

}

void NCSAFontEditorDisplay::adjustLine(const QPoint & p)
{
  qDebug() << buttonGroup->id(buttonGroup->checkedButton());
  int height = info->img.height();
  int width = info->img.width();
  
  if (buttonGroup->id(buttonGroup->checkedButton()) == -2) //TODO code smell!!!!
  {
  
  int left = width+1+info->topLeft.x();
  if( p.x() >= width+1+info->topLeft.x() && p.x() < width+1+info->topLeft.x() + info->img.width())
  {
    left = p.x();
  }
  else if( p.x() >= width+1+info->topLeft.x() + info->img.width())
  {
    left = width+1+info->topLeft.x() + info->img.width();
  }
  
  int maxWidth =  width+1+info->topLeft.x() + info->img.width() - left;

    
  info->width = width > maxWidth ? maxWidth : width;

  info->topLeft.setX(width+1+info->topLeft.x() - left);
  
  }
  else if(buttonGroup->id(buttonGroup->checkedButton()) == -3)
  {
    
    int right = width*2;
    if( p.x() < width+1+info->topLeft.x()+info->img.width())
    {
      right = p.x();
    }
    if(right < width+1)
    {
      right = width + 1;
    }
    info->width = right - width;
  }
  else
  {
    info->topLeft.setY(2*height+info->topLeft.y()-p.y());
  }

  this->repaint();
  
}

void NCSAFontEditorDisplay::mousePressEvent(QMouseEvent * e)
{
  QLabel::mousePressEvent(e);
  emit clicked(e->pos());
}

void NCSAFontEditorDisplay::setFontLetterInfo(FontLetterInfo & lif)
{
  this->info = &lif;
  int height = info->img.height();
  int width = info->img.width();
  
  QPixmap pix(width*3, height*3);
  pix.fill(Qt::transparent);
  this->setPixmap(pix);
  this->adjustSize();

}

void NCSAFontEditorDisplay::paintEvent(QPaintEvent * e)
{
  QLabel::paintEvent(e);
  if(info == NULL)
  {
    return;
  }
  QPainter p(this);
  
  
  int height = info->img.height();
  int width = info->img.width();
  
  p.drawPixmap(width+1+info->topLeft.x(), 2*height+info->topLeft.y(), info->img);
  p.setPen(Qt::yellow);
  p.drawLine(QPoint(0, 2*height), QPoint(3*width, 2*height));
  p.setPen(Qt::blue);
  p.drawLine(QPoint(width, 0), QPoint(width, 3*height));
  p.drawLine(QPoint(width+info->width, 0), QPoint(width+info->width, 3*height));
  
  p.setPen(Qt::green);
}


 void NCSAFontEditor::displayEditorFor(char letter)
 {
   
   if(this->builtFont->count(letter) ==0)
   {
     display->clear();
     return;
   }
   FontLetterInfo& info = (*builtFont)[letter];
   display->setFontLetterInfo(info);
   

 }
 
NCSAFontEditor::NCSAFontEditor(std::map<char, FontLetterInfo> *bf)
{
  this->builtFont = bf;
  
  
  /*
  QScrollArea *scrollArea = new QScrollArea;
  scrollArea->setBackgroundRole(QPalette::Dark);
  scrollArea->setWidget(display);
  scrollArea->resize(100,100);
  */
  QRadioButton * left = new QRadioButton("left bound");
  QRadioButton * right = new QRadioButton("right bound");
  QRadioButton * baseline = new QRadioButton("baseline");
  buttonGroup = new QButtonGroup;
  buttonGroup->addButton(left);
  buttonGroup->addButton(right);
  buttonGroup->addButton(baseline);
  left->setChecked(true);
  display = new NCSAFontEditorDisplay(buttonGroup);
  
  this->addWidget(display);
  this->addWidget(left);
  this->addWidget(right);
  this->addWidget(baseline);
  this->addStretch();
}

#include "ncsafonteditor.moc"