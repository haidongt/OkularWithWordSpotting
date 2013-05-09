/***************************************************************************
*   Copyright (C) 2013 by Haidong Tang                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version eejujmjm2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/

#include "ncsawordspottingutil.h"

#include <tesseract/baseapi.h>
#include <QDebug>
#include <QString>
#include <string>

#include <math.h>
#include <fstream>
#include <set>


NCSASignatureComparator::NCSASignatureComparator()
{

}
NCSASignatureComparator::~NCSASignatureComparator()
{
}
bool NCSASignatureComparator::operator()(NCSAWordInfo* info1, NCSAWordInfo* info2)
{
  if(distance(info1->signature, info1->target) < distance(info2->signature, info1->target))
  {
    return true;
  }
  else
  {
    return false;
  }
}
double NCSASignatureComparator::distance(vector<double> * vec1, vector<double> * vec2)
{
  double distance = 0;
  
  for(int i = 0; i < vec1->size(); i++)
  {
    double diff = (*vec1)[i] - (*vec2)[i];
    distance += diff*diff;
  }
  return distance;
}
  
  
NCSAWordSpottingUtil::NCSAWordSpottingUtil()
{
     wordList = new vector<NCSAWordInfo*>();
}

NCSAWordSpottingUtil::~NCSAWordSpottingUtil()
{
    delete wordList;
}

PIX* NCSAWordSpottingUtil::qImage2PIX(const QImage& originalImage) {
  QImage qImage(originalImage);
  PIX * pixs;
  l_uint32 *lines;

  qImage = qImage.rgbSwapped();
  int width = qImage.width();
  int height = qImage.height();
  int depth = qImage.depth();
  int wpl = qImage.bytesPerLine() / 4;

  pixs = pixCreate(width, height, depth);
  pixSetWpl(pixs, wpl);
  pixSetColormap(pixs, NULL);
  l_uint32 *datas = pixs->data;

  for (int y = 0; y < height; y++) {
    lines = datas + y * wpl;
    QByteArray a((const char*)qImage.scanLine(y), qImage.bytesPerLine());
    for (int j = 0; j < a.size(); j++) {
      *((l_uint8 *)lines + j) = a[j];
    }
  }
  return pixEndianByteSwapNew(pixs);
}

QImage NCSAWordSpottingUtil::PIX2QImage(PIX *pixImage) {
  int width = pixGetWidth(pixImage);
  int height = pixGetHeight(pixImage);
  int depth = pixGetDepth(pixImage);
  int bytesPerLine = pixGetWpl(pixImage) * 4;
  l_uint32 * s_data = pixGetData(pixEndianByteSwapNew(pixImage));

  QImage::Format format;
  if (depth == 1)
    format = QImage::Format_Mono;
  else if (depth == 8)
    format = QImage::Format_Indexed8;
  else
    format = QImage::Format_RGB32;

  QImage result((uchar*)s_data, width, height, bytesPerLine, format);

  // Handle pallete
  QVector<QRgb> _bwCT;
  _bwCT.append(qRgb(255,255,255));
  _bwCT.append(qRgb(0,0,0));

  QVector<QRgb> _grayscaleCT(256);
  for (int i = 0; i < 256; i++)  {
    _grayscaleCT.append(qRgb(i, i, i));
  }
  if (depth == 1) {
    result.setColorTable(_bwCT);
  }  else if (depth == 8)  {
    result.setColorTable(_grayscaleCT);

  } else {
    result.setColorTable(_grayscaleCT);
  }

  if (result.isNull()) {
    static QImage none(0,0,QImage::Format_Invalid);
    qDebug() << "***Invalid format!!!";
    return none;
  }

  return result.rgbSwapped();
}

void NCSAWordSpottingUtil::addPage(const QImage& page, int pagenum)
{
    pageImgs.push_back(page);
    PIX* pix = qImage2PIX(page);
    
    tesseract::TessBaseAPI tess;
    tess.Init(NULL, "eng");
    
    tess.SetImage(pix);
    bool flag;
    int out_offset;
    float out_slope;
    flag = tess.GetTextDirection(&out_offset, &out_slope);
    qDebug() << "slope:" << out_slope;
    pix = pixRotateAM(pix,out_slope,L_BRING_IN_WHITE);
    tess.SetImage(pix);
    tess.GetIterator();
    Boxa* boxa = tess.GetWords(NULL);
    Box** boxes = boxa->box;
    
    for(int i = 0; i < boxa->n; i++)
    {
      PIX* word = pixClipRectangle(pix, boxes[i], NULL); 
      vector<double> * signature = pix2signature(word);
      NCSAWordInfo * word_info = new NCSAWordInfo();
      word_info->box = boxes[i];
      word_info->signature = signature;
      word_info->pagenum = pagenum;
      word_info->page = &(pageImgs.back());
      word_info->width = page.width();
      word_info->height = page.height();
      wordList->push_back(word_info);
      
      std::stringstream ss;
      ss << "/home/htang14/Desktop/okularoutput/bag1/" << i << ".jpg";
      pixWrite(ss.str().c_str(), word, IFF_DEFAULT);
     
    }
  
}

vector<NCSAWordInfo*> NCSAWordSpottingUtil::search(const QPixmap& img, int maxDisplay)
{
    QImage img_QImage = img.toImage();
    PIX* pix = qImage2PIX(img_QImage);
    vector<double> * signature = pix2signature(pix);
    

    /* only for debugging purpose
    std::ofstream datacheck;
    datacheck.open("/home/htang14/Desktop/okularoutput/datacheck.txt");
    outputVec2File(datacheck, signature);
    for(int i = 0; i < wordList->size(); i++)
    {
      outputVec2File(datacheck, wordList->at(i)->signature);
    }
    datacheck.close();
    */
    
        
    std::set<NCSAWordInfo*, NCSASignatureComparator> wordSet;
    std::set<NCSAWordInfo*, NCSASignatureComparator>::iterator iter;

    for(int i = 0; i < wordList->size(); i++)
    {
      qDebug() << i ;
      NCSAWordInfo* info = wordList->at(i);
      info->target = signature;
      info->index = i;
      
      wordSet.insert(info);
    }

    vector<NCSAWordInfo*> output;
    
    
    int count = 0;
    for(iter = wordSet.begin(); iter != wordSet.end() ; iter++)
    {
      count++;
      output.push_back(*iter);
      if(count >= maxDisplay)
      {
	break;
      }
    }

    return output;
}

void NCSAWordSpottingUtil::outputVec2File(std::ofstream &datacheck, vector<double> * signature )
{
  for(int i = 0; i < signature->size(); i++)
    {
      datacheck << signature->at(i) << " ";
    }
    datacheck << "\n";
}


vector<double> * NCSAWordSpottingUtil::pix2signature(PIX* img)
{
      int w = img->w;
      int h = img->h;
      double* image_in_double = new double[w * h];

      for (int j = 0; j < h; j++)
      {
	for (int k = 0; k < w; k++)
	{
	  l_uint8* rgb = (l_uint8*)(&(img->data[j*w+k]));
	  image_in_double[j*w+k] = (rgb[1] + rgb[2] + rgb[3] + 0.0)/3/255;

	}
      }
     
      vector<double> * signature = getSignature_Dimensions(image_in_double, w, h);
      
      delete[] image_in_double;
      return signature;
}


double* NCSAWordSpottingUtil::dct(double* f, int length, int n)
{
  double* dct = new double[n];
  for(int i=0; i<n; i++){
    dct[i] = 0;
    for(int x=0; x<length; x++){
      dct[i] += f[x]*cos((M_PI*(2*x+1)*i)/(2*length));
      
    }
    if(i == 0) 
    {
      dct[i] = 0; 
      //dct[i] *= sqrt(1.0/length);
    }
    else 
    {
      dct[i] *= sqrt(2.0/length);
    }
  }
  
  return dct;
}

vector<double> * NCSAWordSpottingUtil::getSignature_Dimensions(double image[], int w, int h)
{
		
		int c = 10;
		double* signature = new double[3*c];
		int total, top, bottom;
		
		double* p_profile = new double[w];
		double* u_profile = new double[w];
		double* l_profile = new double[w];
		
		for(int x=0; x<w; x++){
			total = 0;
			top = -1;
			bottom = 0;
			
			for(int y=0; y<h; y++){
				if(image[y*w+x] < 0.5){
					total++;
					if(top < 0) top = y;
					bottom = y;
				}
			}
			
			if(top < 0) top = 0;
			
			p_profile[x] = total*1.0/h;
			u_profile[x] = top*1.0/h;
			l_profile[x] = bottom*1.0/h;
			
		}
		
		
		
		//Build signature
		double* p_profile_f = dct(p_profile, w, c);
		double* u_profile_f = dct(u_profile, w, c);
		double* l_profile_f = dct(l_profile, w, c);
		
		vector<double> *signature_vector = new vector<double>();
		for(int i = 0; i < c; i++)
		{
		  signature_vector->push_back(p_profile_f[i]);

		}
		for(int i = 0; i < c; i++)
		{
		  signature_vector->push_back(u_profile_f[i]);
		}
		for(int i = 0; i < c; i++)
		{
		  signature_vector->push_back(l_profile_f[i]);
		}

		
		delete[] p_profile_f;
		delete[] u_profile_f;
		delete[] l_profile_f;
		
		return signature_vector;
}

//#include "ncsafindbar.moc"
