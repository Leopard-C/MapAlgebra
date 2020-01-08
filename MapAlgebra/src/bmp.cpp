#include "bmp.h"
#include "array2D.hpp"
#include <algorithm>

#pragma warning(disable : 4996)

// using uint  = unsigned int;
// using uchar = unsigned char;

Bmp::Bmp(uint bitCount, uint biWidth, uint biHeight) :
	clrTabSize(getClrTabSize_(bitCount)),
	mtxWidth(getMtxWidth_(bitCount, biWidth)),
	mtxHeight(biHeight)
{
	headerFit_(bitCount, biWidth, biHeight);

	if (clrTabSize > 0)
		clrTab = new uchar[clrTabSize];

	// 动态申请二维数组
	if (mtxWidth > 0 && mtxHeight > 0) {
		//mtx = New2D<uchar>(mtxHeight, mtxWidth);
		mtx = new uchar*[mtxHeight];
		for (uint i = 0; i < mtxHeight; ++i) {
			mtx[i] = new uchar[mtxWidth];
		}
	}
}


// 赋值构造函数
Bmp::Bmp(const Bmp& rhs) :
	fileHeader(rhs.fileHeader), infoHeader(rhs.infoHeader),
	clrTabSize(rhs.clrTabSize), mtxWidth(rhs.mtxWidth), mtxHeight(rhs.mtxHeight)
{
	if (rhs.clrTab) {
		clrTab = new uchar[clrTabSize];
		for (uint i = 0; i < clrTabSize; ++i)
			clrTab[i] = rhs.clrTab[i];
	}
	else {
		clrTab = nullptr;
	}

	if (rhs.mtx) {
		mtx = new uchar*[mtxHeight];
		for (uint i = 0; i < mtxHeight; ++i) {
			mtx[i] = new uchar[mtxWidth];
			for (uint j = 0; j < mtxWidth; ++j) {
				mtx[i][j] = rhs.mtx[i][j];
			}
		}
	}
	else {
		mtx = nullptr;
	}
}



// 析构函数
Bmp::~Bmp()
{
	cleanup_();
}

// 从bmp文件读取信息
bool Bmp::read(const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (!fp)
		return false;

	cleanup_();		// 清理原数据

	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

	if (fileHeader.bfType != 0x4D42) {
		fclose(fp);
		cleanup_();
		return false;
	}

	clrTabSize = getClrTabSize_(infoHeader.biBitCount);
	mtxWidth = getMtxWidth_(infoHeader.biBitCount, infoHeader.biWidth);
	mtxHeight = infoHeader.biHeight;

	// 读取调色板
	if (clrTabSize > 0) {
		clrTab = new uchar[clrTabSize];
		fread(clrTab, 1, clrTabSize, fp);
	}

	// 读取像素数据
	uchar* lineBuf = new uchar[mtxWidth];	// 行缓冲
	mtx = new uchar*[mtxHeight];	// 像素矩阵
	for (uint i = 0; i < mtxHeight; ++i) {
		fread(lineBuf, sizeof(uchar), mtxWidth, fp);
		mtx[i] = new uchar[mtxWidth];
		for (uint j = 0; j < mtxWidth; ++j) {
			mtx[i][j] = lineBuf[j];
		}
	}

	fclose(fp);

	if (lineBuf)
		delete[] lineBuf;

	return true;
}


// 写入到文件
bool Bmp::write(const char* filename)
{
	if (!mtx)
		return false;

	FILE* fp = fopen(filename, "wb");
	if (!fp)
		return false;

	// 写入文件头，bmp信息头，调色板
	fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
	fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

	// 是否有调色板
	if (clrTab && clrTabSize > 0)
		fwrite(clrTab, clrTabSize, 1, fp);

	// 写入数据
	for (uint i = 0; i < mtxHeight; ++i)
		fwrite(mtx[i], mtxWidth, 1, fp);

	fclose(fp);
	return true;
}


// 拷贝destMtx到mtx
bool Bmp::copyMtxFrom(uchar** srcMtx, uint srcMtxWidth, uint srcMtxHeight)
{
	if (!(srcMtx && srcMtxWidth > 0 && srcMtxHeight > 0))
		return false;

	if (mtx) {
		if (srcMtxWidth > mtxWidth || srcMtxHeight > mtxHeight)
			return false;
		for (uint i = 0; i < mtxHeight; ++i)
			std::copy(srcMtx[i], srcMtx[i] + srcMtxWidth, mtx[i]);
	}
	else {
		mtxWidth = srcMtxWidth;
		mtxHeight = srcMtxHeight;
		mtx = new uchar*[mtxHeight];
		for (uint i = 0; i < mtxHeight; ++i) {
			mtx[i] = new uchar[mtxWidth];
			std::copy(srcMtx[i], srcMtx[i] + mtxWidth, mtx[i]);
		}
	}

	return true;
}


bool Bmp::copyClrTabFrom(uchar* srcClrTab, uint size)
{
	if (!(srcClrTab && size > 0))
		return false;

	if (!clrTab) {
		clrTabSize = size;
		clrTab = new uchar[clrTabSize];
	}
	else if (size != clrTabSize) {
		return false;
	}

	memcpy(clrTab, srcClrTab, size);
	return true;
}


bool Bmp::add(const Bmp& rhs, float alpha)
{
	uint rhsWidth = rhs.width();
	uint rhsHeight = rhs.height();
	int rhsBitCount = rhs.bitCount();

	if (width() != rhsWidth || height() != rhsHeight || bitCount() != rhsBitCount) {
		return false;
	}

	if (alpha > 1.0f)
		alpha = 1.0f;
	else if (alpha < 0)
		alpha = 0.0;

	switch (bitCount()) {
	default:
		break;
	case 8:
	{
//		for (int y = 0; y < rhsHeight; ++y) {
//			for (int x = 0; x < rhsWidth; ++x) {
//				uchar r = ucharAdd_(clrTab[mtx[y][x] * 4], rhs.clrTab[rhs.mtx[y][x] * 4]);
//				uchar g = ucharAdd_(clrTab[mtx[y][x] * 4 + 1], rhs.clrTab[rhs.mtx[y][x] * 4 + 1]);
//				uchar b = ucharAdd_(clrTab[mtx[y][x] * 4 + 2], rhs.clrTab[rhs.mtx[y][x] * 4 + 2]);
//			}
//		}
		break;
	}
	case 24:
	{
		for (int y = 0; y < rhsHeight; ++y) {
			for (int x = 0; x < rhsWidth; ++x) {
				mtx[y][x * 3] = mtx[y][x * 3] * alpha + rhs.mtx[y][x * 3] * (1 - alpha);
				mtx[y][x * 3 + 1] = mtx[y][x * 3 + 1] * alpha + rhs.mtx[y][x * 3 + 1] * (1 - alpha);
				mtx[y][x * 3 + 2] = mtx[y][x * 3 + 2] * alpha + rhs.mtx[y][x * 3 + 2] * (1 - alpha);
			}
		}
		break;
	}
	case 32:
	{
		for (int y = 0; y < rhsHeight; ++y) {
			for (int x = 0; x < rhsWidth; ++x) {
				mtx[y][x * 4] = mtx[y][x * 4] * alpha + rhs.mtx[y][x * 3] * (1 - alpha);
				mtx[y][x * 4 + 1] = mtx[y][x * 4 + 1] * alpha + rhs.mtx[y][x * 4 + 1] * (1 - alpha);
				mtx[y][x * 4 + 2] = mtx[y][x * 4 + 2] * alpha + rhs.mtx[y][x * 4 + 2] * (1 - alpha);
			}
		}
		break;
	}
	}

	return true;
}

// 叠置处理
bool Bmp::overlay(const Bmp& rhs)
{
	uint rhsWidth = rhs.width();
	uint rhsHeight = rhs.height();
	int rhsBitCount = rhs.bitCount();

	if (width() != rhsWidth || height() != rhsHeight || bitCount() != rhsBitCount) {
		return false;
	}

	switch (bitCount()) {
	default:
		break;
	case 8:
	{
		for (int y = 0; y < rhsHeight; ++y) {
			for (int x = 0; x < rhsWidth; ++x) {
				if (mtx[y][x] != 0xFF && rhs.mtx[y][x] != 0xFF) {
					mtx[y][x] = mtx[y][x] + rhs.mtx[y][x];
				}
				else {
					mtx[y][x] = 0xFF;
				}
			}
		}
		break;
	}
	}

	return true;
}

// 大小相同，位数相同
bool Bmp::isSameSize(const Bmp& rhs) const
{
	return (width() == rhs.width() && height() == rhs.height() && bitCount() == rhs.bitCount());
}

bool Bmp::isSameSize(const Bmp& lhs, const Bmp& rhs)
{
	return (lhs.width() == rhs.width() && lhs.height() == rhs.height() && lhs.bitCount() == rhs.bitCount());
}

// 拷贝mtx到destMtx
bool Bmp::copyMtxTo(uchar*** destMtx, uint* destMtxWidth, uint* destMtxHeight)
{
	if (!mtx || !destMtx || (*destMtx))	// *destMtx 必须为 nullptr
		return false;

	if (destMtxWidth)
		*destMtxWidth = mtxWidth;
	if (destMtxHeight)
		*destMtxHeight = mtxHeight;

	*destMtx = new uchar*[mtxHeight];

	for (uint i = 0; i < mtxHeight; ++i) {
		(*destMtx)[i] = new uchar[mtxWidth];
		std::copy(mtx[i], mtx[i] + mtxWidth, (*destMtx)[i]);
	}

	return true;
}

// 清理 clrTab 和 mtx
void Bmp::cleanup_()
{
	if (clrTab) {
		delete[] clrTab;
		clrTab = nullptr;
	}

	if (mtx) {
		for (uint i = 0; i < mtxHeight; ++i) {
			if (mtx[i]) {
				delete[] mtx[i];
				mtx[i] = nullptr;
			}
		}
		delete[] mtx;
		mtx = nullptr;
	}
}


// 根据bitCount获取调色板大小
uint Bmp::getClrTabSize_(uint biCount) const
{
	uint ret = 0;
	switch (biCount) {
		case 1:	 ret = 0;	 break;
		case 4:  ret = 64;	 break;
		case 8:  ret = 1024; break;
		case 16:
		case 24:
		case 32:
		default: ret = 0;	 break;
	}

	return ret;
}


// 根据biCont和biWidth获取mtxWidth
uint Bmp::getMtxWidth_(uint biCount, uint biWidth) const
{
	uint ret = 0;
	switch (biCount) {
		default:
			break;
		case 1:
			break;
		case 4:
			ret = biWidth % 2 == 0 ?
				(biWidth / 2 + 3) / 4 * 4
				: (biWidth / 2 + 1 + 3) / 4 * 4;
			break;
		case 8:
			ret = (biWidth + 3) / 4 * 4;
			break;
		case 16:
			// ret =
			break;
		case 24:
			ret = (biWidth * 3 + 3) / 4 * 4;
			break;
		case 32:
			ret = biWidth * 4;
			break;
	}

	return ret;
}


// 根据biCount 和 biWidth, biHeight
// 校正infoHeader 和 fileHeader (调整为合适大小)
void Bmp::headerFit_(uint biCount, uint biWidth, uint biHeight)
{
	uint mtxWidth = getMtxWidth_(biCount, biWidth);
	uint mtxHeight = biHeight;

	fileHeader.bfType = 0x4D42;
	fileHeader.bfSize = fileHeader.bfOffBits + mtxWidth * mtxHeight;
	fileHeader.bfOffBits = 14 + 40 + getClrTabSize_(biCount);

	infoHeader.biSize = 40;
	infoHeader.biWidth = biWidth;
	infoHeader.biHeight = biHeight;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = biCount;
	infoHeader.biSizeImage = mtxWidth * mtxHeight;
}


// 两个unsigned char相加，超过255就返回255
uchar Bmp::ucharAdd_(uchar first, uchar second)
{
	int result = int(first) + second;
	return result > 255 ? 255 : first + second;
}

/**********************************************************************************************/

// 灰度化
bool BmpGray(const char* src256ColorBmpName, const char* destBmpName) {
	Bmp bmp;
	bmp.read(src256ColorBmpName);

	if (bmp.bitCount() != 8) {
		printf("Input bmp file must be 256 color (8bit)!\n");
		return false;
	}

	uint colorIdx = 0;
	for (uint i = 0; i < bmp.mtxHeight; ++i) {
		for (uint j = 0; j < bmp.mtxWidth; ++j) {
			colorIdx = bmp.mtx[i][j];
			bmp.mtx[i][j] = (bmp.clrTab[colorIdx * 4] + bmp.clrTab[colorIdx * 4 + 1] + bmp.clrTab[colorIdx * 4 + 2]) / 3;
		}
	}

	// 新的调色板
	for (uint i = 0; i < 256; ++i) {
		bmp.clrTab[i * 4] = i;
		bmp.clrTab[i * 4 + 1] = i;
		bmp.clrTab[i * 4 + 2] = i;
	}

	bmp.write(destBmpName);

	return true;
}

// 二值化
// 输入256色的灰度图像
// @thres 阈值
bool BmpThreshold(const char* srcGrayBmpName, const char* destBmpName, int thres) {
	Bmp bmp;
	bmp.read(srcGrayBmpName);

	if (bmp.bitCount() != 8) {
		printf("Input bmp file must be 256 color (8bit) and gray!!\n");
		return false;
	}

	uint colorIdx = 0;
	for (uint i = 0; i < bmp.mtxHeight; ++i) {
		for (uint j = 0; j < bmp.mtxWidth; ++j) {
			colorIdx = bmp.mtx[i][j];
			if (bmp.clrTab[colorIdx * 4] < thres)
				bmp.mtx[i][j] = 0;
			else
				bmp.mtx[i][j] = 255;
		}
	}

	bmp.write(destBmpName);

	return true;
}


