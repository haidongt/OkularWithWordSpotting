#ifndef __KDVIMULTIPAGE_H
#define __KDVIMULTIPAGE_H


#include <qstringlist.h>


#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <klibloader.h>


class QLabel;


#include <kmultipage.h>


#include "dviwin.h"


class KDVIMultiPageFactory : public KLibFactory
{
  Q_OBJECT

public:

  KDVIMultiPageFactory();
  virtual ~KDVIMultiPageFactory();
 
  virtual QObject* create(QObject* parent = 0, const char* name = 0,
                          const char* classname = "KDVIMultiPage",
                          const QStringList &args = QStringList() );
  
  static KInstance *instance();
 
private:

  static KInstance *s_instance;

};


class KDVIMultiPage : public KMultiPage
{
  Q_OBJECT

public:

  KDVIMultiPage(QWidget *parent=0, const char *name=0);
  virtual ~KDVIMultiPage();


// Interface definition start ------------------------------------------------

  /// returns the list of supported file formats
  virtual QStringList fileFormats();

  /// opens a file
  virtual bool openFile(); 

  /// displays the given page
  virtual bool gotoPage(int page); 

  /// sets a zoom factor
  virtual void setZoom(double z); 

  /// calculates the zoom needed to fit into a given width
  virtual double zoomForWidth(int width); 
  
  /// calculates the zoom needed to fit into a given height
  virtual double zoomForHeight(int height); 

  void setPaperSize(double, double);


signals:

  /// emitted to indicate the number of pages in the file
  void numberOfPages(int nr);

private:

  dviWindow *window;

};


#endif


