/***********************************************************************
**
** class name: Bmp
**
** description: Bmp类，封装了Bitmap文件的读取、写入等基本操作
**
** author:  @github: Leopard-C/MapAlgebra
**
** last change: 2020-01-08
**
************************************************************************/
#pragma once

#include <windows.h>
#include <cstdio>
#include <iostream>

using uint  = unsigned int;
using uchar = unsigned char;

#define LogF(fmt, ...) printf(##fmt"(%s)<%d> <%s>\n", ##__VA_ARGS__, __FILE__, __LINE__, __FUNCTION__) 
#define Log(x) std::cout << x << " (" << __FILE__ << ")<" << __LINE__ << "> <" << __FUNCTION__ << ">" << std::endl;

class Bmp 
{
public:
	Bmp() {}
	Bmp(uint bitCount, uint biWidth, uint biHeight);
	Bmp(const Bmp& rhs);
	~Bmp();

	bool read(const char* filename);
	bool write(const char* filename);
	uint width() const { return infoHeader.biWidth; }
	uint height() const { return infoHeader.biHeight; }
	uint bitCount() const { return infoHeader.biBitCount; }

	bool copyMtxTo(uchar*** destMtx, uint* destMtxWidth, uint* destMtxHeight);		// 拷贝this->mtx到destMtx
	bool copyMtxFrom(uchar** srcMtx, uint destMtxWidth, uint destMtxHeight);		// 拷贝srcMtx到this->mtx
	bool copyClrTabFrom(uchar* srcClrTab, uint size);		// 拷贝srcClrTab到this->clrTab

	bool add(const Bmp& rhs, float alpha = 0.5);	// 图像相加
	bool overlay(const Bmp& rhs);	// 叠置处理 

	bool isSameSize(const Bmp& rhs) const;

	static bool isSameSize(const Bmp& lhs, const Bmp& rhs);

public:
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	uchar* clrTab = nullptr;
	uint clrTabSize = 0;

	uchar** mtx = nullptr;
	uint mtxWidth = 0;
	uint mtxHeight = 0;

private:
	void cleanup_();
	uint getClrTabSize_(uint biCount) const;
	uint getMtxWidth_(uint biCount, uint biWidth) const;
	void headerFit_(uint biCount, uint biWidth, uint biHeight);
	uchar ucharAdd_(uchar first, uchar second);
};


// 灰度化
bool BmpGray(const char* src256ColorBmpName, const char* destBmpName);

// 二值化
bool BmpThreshold(const char* srcGrayBmpName, const char* destBmpName, int thres);

