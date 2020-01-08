/*******************************************************
**
** description: Bmp图像的读取、反色、转换、叠置、平滑
**
** author:  @github: Leopard-C/MapAlgebra
**
** last change: 2020-01-08
**
*******************************************************/
#pragma once

#include <windows.h>
#include <cstdio>

enum InverseMode {
	InverseColorTable = 1,
	InverseColorIndex = 2
};

using uint = unsigned int;
using uchar = unsigned char;


/*************************************************/
/*                                               */
/*       反色处理                                */
/*                                               */
/*************************************************/

// 反色处理，所有bmp类型
bool BmpInverse(const char* srcBmpName, const char* destBmpName, InverseMode mode = InverseColorTable);

//	反色处理（索引值），4bit，16色
bool Bmp16ColorInverse(const char* srcBmpName, const char* destBmpName, InverseMode mode = InverseColorTable);

//	反色处理（索引值），8bit，256色
bool Bmp256ColorInverse(const char* srcBmpName, const char* destBmpName, InverseMode inverseMode = InverseColorTable);

// 反色处理，24bit
bool Bmp24BitInverse(const char* srcBmpName, const char* destBmpName);


/*************************************************/
/*                                               */
/*       转换处理                                */
/*                                               */
/*************************************************/

// 24bit 真彩色 转为 256色
bool Bmp24BitTo256Color(const char* srcBmpName, const char* destBmpName);

// 256色 转 32 bit
bool Bmp256ColorTo32Bit(const char* srcBmpName, const char* destBmpName);


/*************************************************/
/*                                               */
/*       叠置处理                                */
/*                                               */
/*************************************************/

// 图像相加
bool BmpAdd(const char* bmpName1, const char* bmpName2, const char* outputBmpName, float alpha = 0.5);

// 叠置处理
bool BmpOverlay(const char* bmpName1, const char* bmpName2, const char* outputBmpName);

// 二值逻辑叠加
bool BmpBooleanAnd(const char* bmpName1, const char* bmpName2, const char* outputBmpName);
bool BmpBooleanNot(const char* bmpName, const char* outputBmpName);
bool BmpBooleanOr(const char* bmpName1, const char* bmpName2, const char* outputBmpName);
bool BmpBooleanXOr(const char* bmpName1, const char* bmpName2, const char* outputBmpName);


/*************************************************/
/*                                               */
/*       平滑处理                                */
/*                                               */
/*************************************************/

bool BmpSmooth(const char* srcBmpName, const char* destBmpName);

