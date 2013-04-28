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
}

NCSAWordSpottingUtil::~NCSAWordSpottingUtil()
{
}

PIX* NCSAWordSpottingUtil::qImage2PIX(QImage& qImage) {
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

void NCSAWordSpottingUtil::addPage(const QPixmap& page, int pagenum)
{
  
	qDebug() << "In the new class now";

    page.save("/home/htang14/Desktop/okularoutput/12345.png");
    
    QImage img = page.toImage();
    PIX* pix = qImage2PIX(img);
    
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
    
    wordList = new vector<NCSAWordInfo*>();
    for(int i = 0; i < boxa->n; i++)
    {
      PIX* word = pixClipRectangle(pix, boxes[i], NULL); 
      vector<double> * signature = pix2signature(word);
      NCSAWordInfo * word_info = new NCSAWordInfo();
      word_info->box = boxes[i];
      word_info->signature = signature;
      word_info->pagenum = pagenum;
      wordList->push_back(word_info);
     

      /*
      std::stringstream ss;
      ss << "/home/htang14/Desktop/okularoutput/bag/" << i << ".jpg";
      pixWrite(ss.str().c_str(), word, IFF_DEFAULT);
      break;
      */
    }
    qDebug()<<"list size:" << wordList->size();
    //Pix* pixout = pixDrawBoxa(pix,boxa,1,0xaaaaaa);
    
    //pixWrite("/home/htang14/Desktop/okularoutput/123456.jpg", pixout, IFF_DEFAULT);
    //painter_->getPagePixmap(page);
  
}

vector<vector<double> > NCSAWordSpottingUtil::search(const QPixmap& img)
{
    QImage img_QImage = img.toImage();
    PIX* pix = qImage2PIX(img_QImage);
    qDebug()<<"attention please";
    vector<double> * signature = pix2signature(pix);
    qDebug()<<"stop attention";
    std::ofstream datacheck;
    datacheck.open("/home/htang14/Desktop/okularoutput/datacheck.txt");
    
    outputVec2File(datacheck, signature);
    
    for(int i = 0; i < wordList->size(); i++)
    {
      outputVec2File(datacheck, wordList->at(i)->signature);
    }
    datacheck.close();
        
    std::set<NCSAWordInfo*, NCSASignatureComparator> wordSet;
    std::set<NCSAWordInfo*, NCSASignatureComparator>::iterator iter;
    
    for(int i = 0; i < wordList->size(); i++)
    {
      NCSAWordInfo* info = wordList->at(i);
      info->target = signature;
      info->index = i;
      
      wordSet.insert(info);
    }
    
    vector<vector<double> > output;
    
    
    int count = 0;
    for(iter = wordSet.begin(); iter != wordSet.end() || count < 10; iter++)
    {
      count++;
      vector<double> rectangle;
      rectangle.push_back((*iter)->box->x);
      rectangle.push_back((*iter)->box->y);
      rectangle.push_back((*iter)->box->w);
      rectangle.push_back((*iter)->box->h);
      output.push_back(rectangle);
      //qDebug() << (*iter)->index;
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
