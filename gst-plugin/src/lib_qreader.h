/*******************************************************************************
Copyright(R) 2014-2024, MPR Tech. Co., Ltd
File Name: lib_qreader.h
Author: Karsn           Date: 2018-03-15             Version: 1.0
Discription:

Function:

History:
1. Version: 1.0        Date: 2018-03-15         Author: Karsn
    Modification:
      Creation.
*******************************************************************************/

#ifndef __LIB_QREADER_H__
#define __LIB_QREADER_H__

//#ifdef __cplusplus
//extern "C" {
//#endif

struct Tru_Point
{
	float fX;
	float fY;
	int nShape; //0 circle | !0 others
};
typedef struct Tru_Point TruPoint;

struct Tru_Array
{
	TruPoint *pArray;
	int nSize;
};

typedef struct Tru_Array TruArray;

TruArray * qreaderArrayCreate(int nNumOfPoints);
void qreaderArrayDestroy(TruArray * const pArray);
TruArray *qreaderDecode(char *const pImg, int nWidth, int nHeight);


//#ifdef __cplusplus
//}
//#endif

#endif


