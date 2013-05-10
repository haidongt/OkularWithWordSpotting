#include "ncsafonteditor.h"
#include <qdebug.h>
#include <QPainter>
#include <QMouseEvent>

NCSAFontEditorDisplay::NCSAFontEditorDisplay()
{
  info = NULL;
  connect(this, SIGNAL(clicked(const QPoint & )), this, SLOT(testFun(const QPoint & )));

}

void NCSAFontEditorDisplay::testFun(const QPoint & p)
{
  qDebug() << p;
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
  this->resize(width * 3, height * 3);

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
  
  p.drawPixmap(width, height, info->img);
  p.setPen(Qt::blue);
  p.drawLine(QPoint(width, 0), QPoint(width, 3*height));
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
  
  display = new NCSAFontEditorDisplay();
  
  this->addWidget(display);
  this->addStretch();
}

#include "ncsafonteditor.moc"