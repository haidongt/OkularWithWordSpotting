#ifndef _NCSAFONTEDITOR_H_
#define _NCSAFONTEDITOR_H_

#include <QLabel>
#include <qlayout.h>
#include "ncsafindbar.h"
#include <map>
#include <QButtonGroup>
class FontLetterInfo;

class NCSAFontEditorDisplay: public QLabel
{
        Q_OBJECT
public:
  NCSAFontEditorDisplay(QButtonGroup *bg);
  void setFontLetterInfo(FontLetterInfo & lif);
protected:
  void paintEvent(QPaintEvent * e);
  void mousePressEvent(QMouseEvent * e);
private:
  FontLetterInfo * info;
  QButtonGroup *buttonGroup;
signals:
  void clicked(const QPoint & pos);
public slots:
  void adjustLine(const QPoint & p);
};


class NCSAFontEditor: public QVBoxLayout
{
      Q_OBJECT
public:
  NCSAFontEditor(std::map<char, FontLetterInfo> *bf);
  void displayEditorFor(char letter);

private:
  std::map<char, FontLetterInfo> *builtFont;
  NCSAFontEditorDisplay * display;
  QButtonGroup *buttonGroup;

};
#endif
