/***************************************************************************
 *   Copyright (C) 2013 by Haidong Tang                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef _NCSAWORDSPOTTINGUTIL_H_
#define _NCSAWORDSPOTTINGUTIL_H_
#include <QImage>
#include <QPixmap>
#include <leptonica/allheaders.h>
#include <string>
#include <sstream>

#include <vector>
using std::vector;

class NCSAWordInfo
{
public:
  vector<double> * signature;
  Box* box;
  int pagenum;
  vector<double> * target;
  int index;
};

class NCSASignatureComparator
{
public:
  NCSASignatureComparator();
  ~NCSASignatureComparator();
  bool operator()(NCSAWordInfo* info1, NCSAWordInfo* info2);
  
private:
  double distance(vector<double> * vec1, vector<double> * vec2);

  
};

class NCSAWordSpottingUtil
{
    public:
        NCSAWordSpottingUtil();
        ~NCSAWordSpottingUtil();
	void addPage(const QPixmap& page, int pagenum);
	vector<vector<double> > search(const QPixmap& img);
    private:
	PIX* qImage2PIX(QImage& qImage);
	vector<double> * getSignature_Dimensions(double image[], int w, int h);
	double* dct(double* f, int length, int n);
	vector<double> * pix2signature(PIX* img);
	vector<NCSAWordInfo*> *wordList;
	
	void outputVec2File(std::ofstream &datacheck, vector<double> * signature );


};




#endif
