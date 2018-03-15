/*******************************************************************************
Copyright(R) 2014-2024, MPR Tech. Co., Ltd
File Name: lib_qreader.cpp
Author: Karsn           Date: 2018-03-15             Version: 1.0
Discription:

Function:

History:
1. Version: 1.0        Date: 2018-03-15         Author: Karsn
    Modification:
      Creation.
*******************************************************************************/

#include "lib_qreader.h"
#include "ImageReaderSource.h"
#include <zxing/detector/Detector.h>
#include <zxing/common/HybridBinarizer.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>

/****************************** Private typedef *******************************/
/****************************** Private define ********************************/
/****************************** Private macro *********************************/
/***************************** Private variables ******************************/
/************************ Private function prototypes *************************/

/*******************************************************************************
Function: 
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
Return:
Others:
*******************************************************************************/
TruArray *qreaderDecode(char *const pImg, int nWidth, int nHeight)
{
	Ref<LuminanceSource> source;

    try {
      zxing::ArrayRef<char> image = zxing::ArrayRef<char>(pImg, nWidth*nHeight);
      source = new ImageReaderSource(image, nWidth, nHeight, 1);
    } catch (const zxing::IllegalArgumentException &e) {
      cerr << e.what() << " (ignoring)" << endl;
      //continue;
    }
    
    Ref<Binarizer> binarizer;
    binarizer = new HybridBinarizer(source); //implements a local thresholding algorithm
    Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
    
    DecodeHints hints(DecodeHints::DEFAULT_HINT);
    hints.setTryHarder(true);
    
	Detector detector(binary->getBlackMatrix());
	//Ref<DetectorResult> detectorResult(detector.detect(hints));
	//ArrayRef< Ref<ResultPoint> > points (detectorResult->getPoints());
	
	vector<Ref<FinderPattern> > points = detector.detectFindPattern(hints);
	TruVector *lptrv_Vector = (TruVector *)malloc(sizeof(TruVector));
	if(lptrv_Vector == NULL)
	{
		fprintf("%s(): malloc for vecter failed!\r\n", __func__);
		
		return NULL;
	}
	
	lptrv_Vector->pArray = (TruPoint *)malloc(points.size()*sizeof(TruPoint));
	if(lptrv_Vector->pArray == NULL)
	{
		fprintf("%s(): malloc for vecter failed!\r\n", __func__);
		free(lptrv_Vector);
		
		return NULL;
	}
	lptrv_Vector->nSize = points.size();
	
	for (int j = 0; j < points.size(); j++) 
	{
      cout << "  Point[" << j <<  "]: "
           << points[j]->getX() << " "
           << points[j]->getY() << endl;
           
      lptrv_Vector->pArray[j].fX = points[j]->getX();
      lptrv_Vector->pArray[j].fY = points[j]->getY();
    }
	
	return lptrv_Vector;
}
