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
#include <assert.h>
#include <math.h>
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
static int qreaderShapeIdentify(TruArray *const pPatterns, Ref<BitMatrix> image_);

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
 
    try {
      //zxing::ArrayRef<char> image = zxing::ArrayRef<char>(pImg, nWidth*nHeight);
      //source = new ImageReaderSource(image, nWidth, nHeight, 1);
      source = ImageReaderSource::create(pImg, nWidth, nHeight);
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
	if(points.size() == null)
	{
		cout << __func__ << ": None point found" << endl;
		
		return NULL;
	}
	
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
    
    /* Shape Identify */
    qreaderShapeIdentify(lptrv_Array, binary->getBlackMatrix());
    
    /* Only for Debug (Output Binarizer)*/
    Ref<BitMatrix> lclav_BitMatrixRef = binary->getBlackMatrix();
    for(int i=0; i<nHeight; i++)
    {
    	for(int j=0; j< nWidth; j++)
    	{
    		unsigned char lu8v_Gray = 0;
    		lu8v_Gray = lclav_BitMatrixRef->get(j,i) ? 0:255;
    		pImg[(i*nWidth+j)*4] = lu8v_Gray;
    		pImg[(i*nWidth+j)*4+1] = lu8v_Gray;
    		pImg[(i*nWidth+j)*4+2] = lu8v_Gray;
    		pImg[(i*nWidth+j)*4+3] = 0;
    	}
    }
	
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
static int qreaderCheckHorizontal(size_t centerJ, size_t centerI, Ref<BitMatrix> image_) 
{

	int maxJ = image_->getWidth();
	int stateCount[5];
		
	for (int i = 0; i < 5; i++)
		stateCount[i] = 0;
	
	int j = centerJ;
	while (j >= 0 && image_->get(j, centerI)) 
	{
		stateCount[2]++;
		j--;
	}
	if (j < 0) 
	{
		return -1;
	}
	int maxCount = stateCount[2]*2;
	
	while (j >= 0 && !image_->get(j, centerI) && stateCount[1] <= maxCount) 
	{
		stateCount[1]++;
		j--;
	}
	if (j < 0 || stateCount[1] > maxCount) 
	{
		return -1;
	}
	while (j >= 0 && image_->get(j, centerI) && stateCount[0] <= maxCount) 
	{
		stateCount[0]++;
		j--;
	}
	if (stateCount[0] > maxCount) 
	{
		return -1;
	}
	
	j = centerJ + 1;
	while (j < maxJ && image_->get(j, centerI)) 
	{
		stateCount[2]++;
		j++;
	}
	if (j == maxJ) 
	{
		return -1;
	}
	while (j < maxJ && !image_->get(j, centerI) && stateCount[3] < maxCount) 
	{
		stateCount[3]++;
		j++;
	}
	if (j == maxJ || stateCount[3] >= maxCount) 
	{
		return -1;
	}
	while (j < maxJ && image_->get(j, centerI) && stateCount[4] < maxCount) 
	{
		stateCount[4]++;
		j++;
	}
	if (stateCount[4] >= maxCount) 
	{
		return -1;
	}
	
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	
	return stateCountTotal;
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
static int qreaderCheckVertical(size_t centerJ, size_t centerI, Ref<BitMatrix> image_) 
{
	int maxI = image_->getHeight();
	int stateCount[5];
	for (int i = 0; i < 5; i++)
		stateCount[i] = 0;
	
	
	// Start counting up from center
	int i = centerI;
	while (i >= 0 && image_->get(centerJ, i)) 
	{
		stateCount[2]++;
		i--;
	}
	if (i < 0) 
	{
		return -1;
	}
	
	int maxCount = stateCount[2]*2;
	 
	while (i >= 0 && !image_->get(centerJ, i) && stateCount[1] <= maxCount) 
	{
		stateCount[1]++;
		i--;
	}
	// If already too many modules in this state or ran off the edge:
	if (i < 0 || stateCount[1] > maxCount) 
	{
		return -1;
	}
	while (i >= 0 && image_->get(centerJ, i) && stateCount[0] <= maxCount) 
	{
		stateCount[0]++;
		i--;
	}
	if (stateCount[0] > maxCount) 
	{
		return -1;
	}
	
	// Now also count down from center
	i = centerI + 1;
	while (i < maxI && image_->get(centerJ, i)) 
	{
		stateCount[2]++;
		i++;
	}
	if (i == maxI) 
	{
		return -1;
	}
	while (i < maxI && !image_->get(centerJ, i) && stateCount[3] < maxCount) 
	{
		stateCount[3]++;
		i++;
	}
	if (i == maxI || stateCount[3] >= maxCount) 
	{
		return -1;
	}
	while (i < maxI && image_->get(centerJ, i) && stateCount[4] < maxCount) 
	{
		stateCount[4]++;
		i++;
	}
	if (stateCount[4] >= maxCount) 
	{
		return -1;
	}
	
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	
	return stateCountTotal;
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
//int const ls32c_YDelt[] = {0,2,3,5,7,9,10,12,14,16,17,19};

static int round_double(float number)
{
    return (number > 0.0) ? (number + 0.5) : (number - 0.5); 
}

static int qreaderCheck30(size_t centerJ, size_t centerI, Ref<BitMatrix> image_) 
{
	int maxI = image_->getHeight();
	int maxJ = image_->getWidth();
	int stateCount[5];
	for (int i = 0; i < 5; i++)
		stateCount[i] = 0;
	
	
	// Start counting up from center
	int i = centerI;
	int j = centerJ;
	while((i >= 0) && (j>=0) && image_->get(j, i)) 
	{
		stateCount[2]++;
		i--;
		//j=round(1.732f*((float)i-centerI))+centerJ;
		j = (int)round(1.732f*((float)i-centerI)) + centerJ;
	}
	if((i < 0) || (j<0))
	{
		cout << __func__ << ": 2_0 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	int maxCount = stateCount[2]*2;
	
	while((i >= 0) && (j>=0) && (!image_->get(j, i)) && (stateCount[1] <= maxCount)) 
	{
		stateCount[1]++;
		i--;
		j = (int)round(1.732f*((float)i-centerI)) + centerJ;
	}
	// If already too many modules in this state or ran off the edge:
	if((i < 0) || (j<0) || (stateCount[1] > maxCount)) 
	{
		cout << __func__ << ": 1 error -- " << i << "," << j <<endl;
		
		return -1;
	}
	while((i >= 0) && (j>=0) && image_->get(j, i) && (stateCount[0] <= maxCount))
	{
		stateCount[0]++;
		i--;
		j = (int)round(1.732f*((float)i-centerI)) + centerJ;
	}
	if((i < 0) || (j<0) || (stateCount[0] > maxCount))
	{
		cout << __func__ << ": 0 error -- " << i << "," << j <<endl;
		
		return -1;
	}
	
	// Now also count down from center
	i = centerI + 1;
	j = (int)round(1.732f*((float)i-centerI)) + centerJ;
	while((i < maxI) && (j<maxJ) && image_->get(j, i)) 
	{
		stateCount[2]++;
		i++;
		j = (int)round(1.732f*((float)i-centerI)) + centerJ;
	}
	if ((i >= maxI) || (j>=maxJ)) 
	{
		cout << __func__ << ": 2_1 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i < maxI) && (j<maxJ) && (!image_->get(j, i)) && (stateCount[3] < maxCount))
	{
		stateCount[3]++;
		i++;
		j = (int)round(1.732f*((float)i-centerI)) + centerJ;
	}
	if((i >= maxI) || (j >= maxJ) || (stateCount[3] >= maxCount)) 
	{
		cout << __func__ << ": 3 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i < maxI) && (j<maxJ) && image_->get(j, i) && (stateCount[4] < maxCount))
	{
		stateCount[4]++;
		i++;
		j = (int)round(1.732f*((float)i-centerI)) + centerJ;
	}
	if (stateCount[4] >= maxCount) 
	{
		cout << __func__ << ": 4 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	
	return (stateCountTotal*2);
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
static int qreaderCheck60(size_t centerJ, size_t centerI, Ref<BitMatrix> image_) 
{
	int maxI = image_->getHeight();
	int maxJ = image_->getWidth();
	int stateCount[5];
	for (int i = 0; i < 5; i++)
		stateCount[i] = 0;
	
	
	// Start counting up from center
	int i = centerI;
	int j = centerJ;
	while((i >= 0) && (j >= 0) && image_->get(j, i)) 
	{
		stateCount[2]++;
		j--;
		i=(int)round(1.732f*((float)j-centerJ)) + centerI;
	}
	if((i < 0) || (j<0))
	{
		cout << __func__ << ": 2_0 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	int maxCount = stateCount[2]*2;
	
	while((i >= 0) && (j>=0) && (!image_->get(j, i)) && (stateCount[1] <= maxCount)) 
	{
		stateCount[1]++;
		j--;
		i=(int)round(1.732f*((float)j-centerJ)) + centerI;
	}
	// If already too many modules in this state or ran off the edge:
	if((i < 0) || (j<0) || (stateCount[1] > maxCount)) 
	{
		cout << __func__ << ": 1 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i >= 0) && (j>=0) && image_->get(j, i) && (stateCount[0] <= maxCount))
	{
		stateCount[0]++;
		j--;
		i=(int)round(1.732f*((float)j-centerJ)) + centerI;
	}
	if((i < 0) || (j<0) || (stateCount[0] > maxCount))
	{
		cout << __func__ << ": 0 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	// Now also count down from center
	j = centerJ + 1;
	i=(int)round(1.732f*((float)j-centerJ)) + centerI;
	while((i < maxI) && (j<maxJ) && image_->get(j, i)) 
	{
		stateCount[2]++;
		j++;
		i=(int)round(1.732f*((float)j-centerJ)) + centerI;
	}
	if ((i >= maxI) || (j>=maxJ)) 
	{
		cout << __func__ << ": 2_1 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i < maxI) && (j<maxJ) && (!image_->get(j, i)) && (stateCount[3] < maxCount))
	{
		stateCount[3]++;
		j++;
		i=(int)round(1.732f*((float)j-centerJ)) + centerI;
	}
	if((i >= maxI) || (j >= maxJ) || (stateCount[3] >= maxCount)) 
	{
		cout << __func__ << ": 3 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i < maxI) && (j<maxJ) && image_->get(j, i) && (stateCount[4] < maxCount))
	{
		stateCount[4]++;
		j++;
		i=(int)round(1.732f*((float)j-centerJ)) + centerI;
	}
	if (stateCount[4] >= maxCount) 
	{
		cout << __func__ << ": 4 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	
	return (stateCountTotal*2);
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
static int qreaderCheck150(size_t centerJ, size_t centerI, Ref<BitMatrix> image_) 
{
	int maxI = image_->getHeight();
	int maxJ = image_->getWidth();
	int stateCount[5];
	for (int i = 0; i < 5; i++)
		stateCount[i] = 0;
	
	
	// Start counting up from center
	int i = centerI;
	int j = centerJ;
	while((i >= 0) && (j<maxJ) && image_->get(j, i)) 
	{
		stateCount[2]++;
		i--;
		j=(int)round(1.732f*((float)centerI-i)) + centerJ;
	}
	if((i < 0) || (j >= maxJ))
	{
		cout << __func__ << ": 2_0 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	int maxCount = stateCount[2]*2;
	
	while((i >= 0) && (j<maxJ) && (!image_->get(j, i)) && (stateCount[1] <= maxCount)) 
	{
		stateCount[1]++;
		i--;
		j=(int)round(1.732f*((float)centerI-i)) + centerJ;
	}
	// If already too many modules in this state or ran off the edge:
	if((i < 0) || (j >= maxJ) || (stateCount[1] > maxCount)) 
	{
		cout << __func__ << ": 1 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i >= 0) && (j<maxJ) && image_->get(j, i) && (stateCount[0] <= maxCount))
	{
		stateCount[0]++;
		i--;
		j=(int)round(1.732f*((float)centerI-i)) + centerJ;
	}
	if((i < 0) || (j >= maxJ) || (stateCount[0] > maxCount))
	{
		cout << __func__ << ": 0 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	// Now also count down from center
	i = centerI + 1;
	j=(int)round(1.732f*((float)centerI-i)) + centerJ;
	while((i < maxI) && (j>=0) && image_->get(j, i)) 
	{
		stateCount[2]++;
		i++;
		j=(int)round(1.732f*((float)centerI-i)) + centerJ;
	}
	if ((i >= maxI) || (j<0)) 
	{
		cout << __func__ << ": 2_1 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i < maxI) && (j>=0) && (!image_->get(j, i)) && (stateCount[3] < maxCount))
	{
		stateCount[3]++;
		i++;
		j=(int)round(1.732f*((float)centerI-i)) + centerJ;
	}
	if((i >= maxI) || (j<0) || (stateCount[3] >= maxCount)) 
	{
		cout << __func__ << ": 3 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i < maxI) && (j>=0) && image_->get(j, i) && (stateCount[4] < maxCount))
	{
		stateCount[4]++;
		i++;
		j=(int)round(1.732f*((float)centerI-i)) + centerJ;
	}
	if (stateCount[4] >= maxCount) 
	{
		cout << __func__ << ": 4 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	
	return (stateCountTotal*2);
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
static int qreaderCheck120(size_t centerJ, size_t centerI, Ref<BitMatrix> image_) 
{
	int maxI = image_->getHeight();
	int maxJ = image_->getWidth();
	int stateCount[5];
	for (int i = 0; i < 5; i++)
		stateCount[i] = 0;
	
	
	// Start counting up from center
	int i = centerI;
	int j = centerJ;
	while((i >= 0) && (j<maxJ) && image_->get(j, i)) 
	{
		stateCount[2]++;
		j++;
		i=(int)round(1.732f*((float)centerJ-j)) + centerI;
	}
	if((i < 0) || (j>=maxJ))
	{
		cout << __func__ << ": 2_0 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	int maxCount = stateCount[2]*2;
	
	while((i >= 0) && (j<maxJ) && (!image_->get(j, i)) && (stateCount[1] <= maxCount)) 
	{
		stateCount[1]++;
		j++;
		i=(int)round(1.732f*((float)centerJ-j)) + centerI;
	}
	// If already too many modules in this state or ran off the edge:
	if((i < 0) || (j>=maxJ) || (stateCount[1] > maxCount)) 
	{
		cout << __func__ << ": 1 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i >= 0) && (j<maxJ) && image_->get(j, i) && (stateCount[0] <= maxCount))
	{
		stateCount[0]++;
		j++;
		i=(int)round(1.732f*((float)centerJ-j)) + centerI;
	}
	if((i < 0) || (j>=maxJ) || (stateCount[0] > maxCount))
	{
		cout << __func__ << ": 0 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	// Now also count down from center
	j = centerJ - 1;
	i=(int)round(1.732f*((float)centerJ-j)) + centerI;
	while((i < maxI) && (j>=0) && image_->get(j, i)) 
	{
		stateCount[2]++;
		j++;
		i=(int)round(1.732f*((float)centerJ-j)) + centerI;
	}
	if ((i >= maxI) || (j<0)) 
	{
		cout << __func__ << ": 2_1 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i < maxI) && (j>=0) && (!image_->get(j, i)) && (stateCount[3] < maxCount))
	{
		stateCount[3]++;
		j++;
		i=(int)round(1.732f*((float)centerJ-j)) + centerI;
	}
	if((i >= maxI) || (j <0) || (stateCount[3] >= maxCount)) 
	{
		cout << __func__ << ": 3 error -- " << i << "," << j <<endl;
		return -1;
	}
	while((i < maxI) && (j>=0) && image_->get(j, i) && (stateCount[4] < maxCount))
	{
		stateCount[4]++;
		j++;
		i=(int)round(1.732f*((float)centerJ-j)) + centerI;
	}
	if (stateCount[4] >= maxCount) 
	{
		cout << __func__ << ": 4 error -- " << i << "," << j <<endl;
		return -1;
	}
	
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	
	return (stateCountTotal*2);
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
static int qreaderCheck45(size_t centerJ, size_t centerI, Ref<BitMatrix> image_) 
{
	int maxI = image_->getHeight();
	int maxJ = image_->getWidth();
	int stateCount[5];
	for (int i = 0; i < 5; i++)
		stateCount[i] = 0;
	
	
	// Start counting up from center
	int i = centerI;
	int j = centerJ;
	while((i >= 0) && (j >= 0) && image_->get(j, i)) 
	{
		stateCount[2]++;
		i--;
		j--;
	}
	if((i < 0) || (j<0))
	{
		return -1;
	}
	
	int maxCount = stateCount[2]*2;
	
	while((i >= 0) && (j>= 0) && (!image_->get(j, i)) && (stateCount[1] <= maxCount)) 
	{
		stateCount[1]++;
		i--;
		j--;
	}
	// If already too many modules in this state or ran off the edge:
	if((i < 0) || (j<0) || (stateCount[1] > maxCount)) 
	{
		return -1;
	}
	while((i >= 0) && (j>= 0) && image_->get(j, i) && (stateCount[0] <= maxCount))
	{
		stateCount[0]++;
		i--;
		j--;
	}
	if((i < 0) || (j<0) || (stateCount[0] > maxCount))
	{
		return -1;
	}
	
	// Now also count down from center
	i = centerI + 1;
	j = centerJ + 1;
	while((i < maxI) && (j<maxJ) && image_->get(j, i)) 
	{
		stateCount[2]++;
		i++;
		j++;
	}
	if ((i >= maxI) || (j>=maxJ)) 
	{
		return -1;
	}
	while((i < maxI) && (j<maxJ) && (!image_->get(j, i)) && (stateCount[3] < maxCount))
	{
		stateCount[3]++;
		i++;
		j++;
	}
	if((i >= maxI) || (j>=maxJ) || (stateCount[3] >= maxCount)) 
	{
		return -1;
	}
	while((i < maxI) && (j<maxJ) && image_->get(j, i) && (stateCount[4] < maxCount))
	{
		stateCount[4]++;
		i++;
		j++;
	}
	if((i>=maxI) || (j>=maxJ) || (stateCount[4] >= maxCount)) 
	{
		return -1;
	}
	
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	
	return (stateCountTotal*1414/1000);
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
static int qreaderCheck135(size_t centerJ, size_t centerI, Ref<BitMatrix> image_) 
{
	int maxI = image_->getHeight();
	int maxJ = image_->getWidth();
	int stateCount[5];
	for (int i = 0; i < 5; i++)
		stateCount[i] = 0;
	
	
	// Start counting up from center
	int i = centerI;
	int j = centerJ;
	while((i >= 0) && (j < maxJ) && image_->get(j, i)) 
	{
		stateCount[2]++;
		i--;
		j++;
	}
	if((i < 0) || (j>maxJ))
	{
		return -1;
	}
	
	int maxCount = stateCount[2]*2;
	
	while((i >= 0) && (j<maxJ) && (!image_->get(j, i)) && (stateCount[1] <= maxCount)) 
	{
		stateCount[1]++;
		i--;
		j++;
	}
	// If already too many modules in this state or ran off the edge:
	if((i < 0) || (j>maxJ) || (stateCount[1] > maxCount)) 
	{
		return -1;
	}
	while((i >= 0) && (j<maxJ) && image_->get(j, i) && (stateCount[0] <= maxCount))
	{
		stateCount[0]++;
		i--;
		j++;
	}
	if((i < 0) || (j>maxJ) || (stateCount[0] > maxCount))
	{
		return -1;
	}
	
	// Now also count down from center
	i = centerI + 1;
	j = centerJ - 1;
	while((i < maxI) && (j>=0) && image_->get(j, i)) 
	{
		stateCount[2]++;
		i++;
		j--;
	}
	if ((i >= maxI) || (j<0)) 
	{
		return -1;
	}
	while((i < maxI) && (j>=0) && (!image_->get(j, i)) && (stateCount[3] < maxCount))
	{
		stateCount[3]++;
		i++;
		j--;
	}
	if((i >= maxI) || (j<0) || (stateCount[3] >= maxCount)) 
	{
		return -1;
	}
	while((i < maxI) && (j>=0) && image_->get(j, i) && (stateCount[4] < maxCount))
	{
		stateCount[4]++;
		i++;
		j--;
	}
	if((i>=maxI) || (j<0) || (stateCount[4] >= maxCount)) 
	{
		return -1;
	}
	
	int stateCountTotal = stateCount[0] + stateCount[1] + stateCount[2] + stateCount[3] + stateCount[4];
	
	return (stateCountTotal*1414/1000);
}


#if 0
/*******************************************************************************
Function: qreaderShapeIdentify
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
Return:
 1.[int]
   <0 failed
   =0 circle
   =1 not circle
Others:
*******************************************************************************/
#define CIRCLE_THRESHOLD 0.3f
static int qreaderShapeCheck(ArrayRef<int> cDimensions)
{
	/* Check input */
	assert(cDimensions->size() == 4);
	
	/* Cal Mean */
	int ls32v_Mean = 0;
	for(int i=0; i<4; i++)
	{
		ls32v_Mean += cDimensions[i];
	}
	ls32v_Mean /= 5;
	
	/* Cal Standard Deviation */
	int ls32v_MDevi = 0;
	for(int i=0; i<4; i++)
	{
		int ls32v_Diff = (cDimensions[i]-ls32v_Mean);
		ls32v_MDevi += ls32v_Diff*ls32v_Diff;
	}
	ls32v_MDevi /= 4;
	float lf32v_MDeviNorm = sqrt(ls32v_MDevi)/ls32v_Mean;
	
	cout << __func__ << ": Std Devi(Norm),Mean = " << lf32v_MDeviNorm << "," << ls32v_Mean << endl;
	
	if(ls32v_MDevi > CIRCLE_THRESHOLD)
	{
		return 1;
	}

	return 0;
}

/*******************************************************************************
Function: qreaderShapeIdentify
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
 1.[pPatterns]
   Fill nShape segment;
Return:
 1.[int]
   <0 failed
   >=0 succcess
Others:
*******************************************************************************/
static int qreaderShapeIdentify(TruArray *const pPatterns, Ref<BitMatrix> image_)
{
	/* Check Input */
	assert(pPatterns != NULL);
	assert(image_ != NULL);
	
	/* handle */
	for(int i=0; i<pPatterns->nSize; i++)
	{
		ArrayRef<int> lclav_Dimensions(4);
		
		/* 0 */
		lclav_Dimensions[0] = qreaderCheckHorizontal((size_t)(pPatterns->pArray[i].fX), 
		                                             (size_t)(pPatterns->pArray[i].fY), 
		                                             image_);
		
		if(lclav_Dimensions[0] < 0)
		{
			pPatterns->pArray[i].nShape = 1;
			continue;
		}
		
		/* 30 */
		lclav_Dimensions[1] = qreaderCheck30((size_t)(pPatterns->pArray[i].fX), 
		                                     (size_t)(pPatterns->pArray[i].fY), 
		                                      image_);
		
		if(lclav_Dimensions[1] < 0)
		{
			pPatterns->pArray[i].nShape = 1;
			continue;
		}
		
		/* 90 */
		lclav_Dimensions[2] = qreaderCheckVertical((size_t)(pPatterns->pArray[i].fX), 
		                                           (size_t)(pPatterns->pArray[i].fY), 
		                                           image_);
		
		if(lclav_Dimensions[2] < 0)
		{
			pPatterns->pArray[i].nShape = 1;
			continue;
		}
		
		/* 135 */
		lclav_Dimensions[3] = qreaderCheck135((size_t)(pPatterns->pArray[i].fX), 
		                                      (size_t)(pPatterns->pArray[i].fY), 
		                                       image_);
		
		if(lclav_Dimensions[3] < 0)
		{
			pPatterns->pArray[i].nShape = 1;
			continue;
		}
		
		pPatterns->pArray[i].nShape = qreaderShapeCheck(lclav_Dimensions);
	}
	
	return 0;
}
#endif

/*******************************************************************************
Function: qreaderShapeIdentify
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
Return:
 1.[int]
   <0 failed
   =0 circle
   =1 not circle
Others:
*******************************************************************************/
#define CIRCLE_THRESHOLD 0.3f
	
static int qreaderShapeCheck(ArrayRef<int> cDimensions)
{
	float const lclac_CosVect[]={1,-0.5,-0.5,1,-0.5,-0.5};
	float const lclac_SinVect[]={0,0.866025404,-0.866025404,0,0.866025404,-0.866025404};
	
	/* Check input */
	assert(cDimensions->size() == (sizeof(lclac_CosVect)/sizeof(float)));
	
	/* Cal Mean */
	int ls32v_Mean = 0;
	for(int i=0; i<cDimensions->size(); i++)
	{
		ls32v_Mean += cDimensions[i];
	}
	ls32v_Mean /= cDimensions->size();
	
	/* Cal Vect */
	float lf32v_CosVect = 0;
	float lf32v_SinVect = 0;
	for(int i=0; i<cDimensions->size(); i++)
	{
		lf32v_CosVect += cDimensions[i]*lclac_CosVect[i];
		lf32v_SinVect += cDimensions[i]*lclac_SinVect[i];
	}
	
	lf32v_CosVect /= ls32v_Mean;
	lf32v_SinVect /= ls32v_Mean;
	
	
	/* Cal Standard Deviation */
	float lf32v_Scope = sqrt(lf32v_CosVect*lf32v_CosVect + lf32v_SinVect*lf32v_SinVect);
	
	cout << __func__ << ": (cos,sin,scope) = " << lf32v_CosVect << "," << lf32v_SinVect << "," << lf32v_Scope << endl;
	
	if(lf32v_Scope > CIRCLE_THRESHOLD)
	{
		return 1;
	}

	return 0;
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
static int qreaderShapeIdentify(TruArray *const pPatterns, Ref<BitMatrix> image_)
{
	/* Check Input */
	assert(pPatterns != NULL);
	assert(image_ != NULL);
	
	/* handle */
	for(int i=0; i<pPatterns->nSize; i++)
	{
		ArrayRef<int> lclav_Dimensions(6);
		
		/* 0 */
		lclav_Dimensions[0] = qreaderCheckHorizontal((size_t)(pPatterns->pArray[i].fX), 
		                                             (size_t)(pPatterns->pArray[i].fY), 
		                                             image_);
		
		if(lclav_Dimensions[0] < 0)
		{
			pPatterns->pArray[i].nShape = 1;
			continue;
		}
		
		cout << "Dim[6]=" << lclav_Dimensions[0] << ",";
		
		/* 30 */
		lclav_Dimensions[1] = qreaderCheck30((size_t)(pPatterns->pArray[i].fX), 
		                                     (size_t)(pPatterns->pArray[i].fY), 
		                                      image_);
		
		if(lclav_Dimensions[1] < 0)
		{
			pPatterns->pArray[i].nShape = 1;
			continue;
		}
		
		cout << lclav_Dimensions[1] << ",";
		
		/* 60 */
		lclav_Dimensions[2] = qreaderCheck60((size_t)(pPatterns->pArray[i].fX), 
		                                     (size_t)(pPatterns->pArray[i].fY), 
		                                      image_);
		
		if(lclav_Dimensions[2] < 0)
		{
			pPatterns->pArray[i].nShape = 1;
			continue;
		}
		
		cout << lclav_Dimensions[2] << ",";
		
		/* 90 */
		lclav_Dimensions[3] = qreaderCheckVertical((size_t)(pPatterns->pArray[i].fX), 
		                                           (size_t)(pPatterns->pArray[i].fY), 
		                                           image_);
		
		if(lclav_Dimensions[3] < 0)
		{
			pPatterns->pArray[i].nShape = 1;
			continue;
		}
		
		cout << lclav_Dimensions[3] << ",";
		
		/* 120 */
		lclav_Dimensions[4] = qreaderCheck120((size_t)(pPatterns->pArray[i].fX), 
		                                      (size_t)(pPatterns->pArray[i].fY), 
		                                       image_);
		
		if(lclav_Dimensions[4] < 0)
		{
			pPatterns->pArray[i].nShape = 1;
			continue;
		}
		
		cout << lclav_Dimensions[4] << ",";
		
		/* 150 */
		lclav_Dimensions[5] = qreaderCheck150((size_t)(pPatterns->pArray[i].fX), 
		                                      (size_t)(pPatterns->pArray[i].fY), 
		                                       image_);
		
		if(lclav_Dimensions[5] < 0)
		{
			pPatterns->pArray[i].nShape = 1;
			continue;
		}
		
		cout << lclav_Dimensions[5] << endl;
		
		pPatterns->pArray[i].nShape = qreaderShapeCheck(lclav_Dimensions);
	}
	
	return 0;
}