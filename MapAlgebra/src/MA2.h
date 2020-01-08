/*******************************************************
**
** description: 距离变换、提取缓冲区
**
** author:  @github: Leopard-C/MapAlgebra
**
** last change: 2020-12-09
**
*******************************************************/
#pragma once

#include <math.h>

#include "dis_template.hpp"

class Bmp;

// 256色位图距离变换
// maxColor = 0xFF 外距变换
// maxColor = 0x00 内距变换
bool DisTrans256(const char* srcBmpName, DistanceTemplate* pDisT, const char* disBmpName, const char* locBmpName, uchar maxColor = 0xFF);

// 32bit位图距离变换
bool DisTrans32bit(const char* srcBmpName, DistanceTemplate* pDisT, const char* disBmpName, const char* locBmpName, uchar maxColor = 0xFF);

// 提取边界
bool GetBoundaryFromBmp(const char* bmpInputName, const char* bmpOutputName);
bool GetBoundary(const uchar* const * srcMtx, uint mtxWidth, uint mtxHeight, uchar*** boundaryMtx);

// 提取缓冲区
bool GetBufferFromBmp(const char* disBmpName, const char* locBmpName, const char* bmpOutputBufName, float radius);
bool GetBuffer(const float* const* disMtx, uint mtxWidth, uint mtxHeight, uchar*** bufferMtx, float radius);

