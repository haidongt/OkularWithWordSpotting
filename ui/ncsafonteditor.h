#ifndef _NCSAFONTEDITOR_H_
#define _NCSAFONTEDITOR_H_

#include <QLabel>
#include <qlayout.h>
#include "ncsafindbar.h"
#include <map>

class FontLetterInfo;

class NCSAFontEditorDisplay: public QLabel
{
        Q_OBJECT
public:
  NCSAFontEditorDisplay();
  void setFontLetterInfo(FontLetterInfo & lif);
protected:
  void paintEvent(QPaintEvent * e);
  void mousePressEvent(QMouseEvent * e);
private:
  FontLetterInfo * info;
signals:
  void clicked(const QPoint & pos);
public slots:
  void testFun(const QPoint & p);
};

class NCSAFontEditor: public QHBoxLayout
{
      Q_OBJECT
public:
  NCSAFontEditor(std::map<char, FontLetterInfo> *bf);
  void displayEditorFor(char letter);

private:
  std::map<char, FontLetterInfo> *builtFont;
  NCSAFontEditorDisplay * display;
};
#endif
