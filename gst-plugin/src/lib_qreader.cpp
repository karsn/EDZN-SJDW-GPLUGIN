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

//#include <stddef.h>
#include "lib_qreader.h"
#include "ImageReaderSource.h"
#include <zxing/detector/Detector.h>
#include <zxing/common/HybridBinarizer.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>

using namespace std;
using namespace zxing;
//using namespace zxing::multi;
using namespace zxing::qrcode;

/****************************** Private typedef *******************************/
/****************************** Private define ********************************/
#define null 0

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
TruArray * qreaderArrayCreate(int nNumOfPoints)
{
	/* Check Input */
	if(nNumOfPoints == null)
	{
		printf("%s(): invalid input!\r\n", __func__);
		
		return NULL;
	}
	
	/* Create Array */
	TruArray *lptrv_Array = (TruArray *)malloc(sizeof(TruArray));
	if(lptrv_Array == NULL)
	{
		printf("%s(): Create array failed!\r\n", __func__);
		
		return NULL;
	}
	
	lptrv_Array->pArray = (TruPoint *)malloc(nNumOfPoints*sizeof(TruPoint));
	if(lptrv_Array->pArray == NULL)
	{
		printf("%s(): malloc for pArray failed!\r\n", __func__);
		free(lptrv_Array);
		
		return NULL;
	}
	lptrv_Array->nSize = nNumOfPoints;
	
	return lptrv_Array;
}
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
void qreaderArrayDestroy(TruArray * const pArray)
{
	/* Check Input */
	if(pArray == NULL)
	{
		printf("%s(): input NULL pointer\r\n",__func__);
		return;
	}
	
	/* Free */
	if(pArray->pArray != NULL)
	{
		free(pArray->pArray);
	}
	
	free(pArray);
}
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

	cout << __func__ << ": start..." << endl;    
    try {
      //zxing::ArrayRef<char> image = zxing::ArrayRef<char>(pImg, nWidth*nHeight);
      //source = new ImageReaderSource(image, nWidth, nHeight, 1);
      source = ImageReaderSource::create(pImg, nWidth, nHeight);
    } catch (const zxing::IllegalArgumentException &e) {
      cerr << e.what() << " (ignoring)" << endl;
      //continue;
    }
    
    cout << __func__ << ": create img success" << endl;
    
    Ref<Binarizer> binarizer;
    binarizer = new HybridBinarizer(source); //implements a local thresholding algorithm
    Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
    
    cout << __func__ << ": Binarizer success" << endl;
    
    DecodeHints hints(DecodeHints::DEFAULT_HINT);
    hints.setTryHarder(true);
    
    cout << __func__ << ": Set hints success" << endl;
    
    Ref<BitMatrix> lptrv_BitMatrix = binary->getBlackMatrix();
    cout << __func__ << ": Get BlackMatrix success" << endl;
    Detector detector(lptrv_BitMatrix);

	//Detector detector(binary->getBlackMatrix());
	//Ref<DetectorResult> detectorResult(detector.detect(hints));
	//ArrayRef< Ref<ResultPoint> > points (detectorResult->getPoints());
	
	cout << __func__ << ": detector success" << endl;
	
	vector<Ref<FinderPattern> > points = detector.detectFindPattern(hints);
	TruArray *lptrv_Array = qreaderArrayCreate(points.size());
	if(lptrv_Array == NULL)
	{
		//printf("%s(): Create array failed!\r\n",__func__);
		cout << __func__ << ": Create array failed!" << endl;
		
		return NULL;
	}
	
	for (int j = 0; j < points.size(); j++) 
	{
      cout << "  Point[" << j <<  "]: "
           << points[j]->getX() << " "
           << points[j]->getY() << endl;
           
      lptrv_Array->pArray[j].fX = points[j]->getX();
      lptrv_Array->pArray[j].fY = points[j]->getY();
    }
	
	return lptrv_Array;
}
