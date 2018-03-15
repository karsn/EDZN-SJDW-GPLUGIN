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

struct Tru_Point
{
	float fX;
	float fY;
};

struct Tru_Array
{
	struct TruPoint *pArray;
	int nSize;
} ;
typedef struct Tru_Point TruPoint;
typedef struct Tru_Array TruArray;

TruArray *qreaderDecode(char *const pImg, int nWidth, int nHeight);

#endif


