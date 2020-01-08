
#include "MA2.h"
#include "bmp.h"
#include "MA1.h"
#include "array2D.hpp"

#include <algorithm>
#include <set>
#include <vector>

#pragma warning(disable : 4996)

#define MaxFloat 9999999999999.0
#define MinFloat 0.00000001

#define IsZero(x)  fabs(x) < MinFloat
#define IsEqual(x, y) IsZero(x - y)


bool DisTrans256(const char* srcBmpName, DistanceTemplate* pDisT, const char* disBmpName, const char* locBmpName, uchar maxColor/* = 0xFF*/)
{
	Bmp bmpSrc;
	if (!bmpSrc.read(srcBmpName)) {
		LogF("Read %s errror", srcBmpName);
		return false;
	}

	uint bmpWidth = bmpSrc.width();
	uint bmpHeight = bmpSrc.height();
	uint mtxHeight = bmpSrc.mtxHeight;
	uint mtxWidth = bmpSrc.mtxWidth;

	// 分配场
	uchar** locMtx = nullptr;
	bmpSrc.copyMtxTo(&locMtx, nullptr, nullptr);

	// 距离场
	uchar** disMtx = New2D<uchar>(bmpHeight, mtxWidth);

	// 根据分配场的值初始化距离矩阵
	for (uint y = 0; y < bmpHeight; ++y) {
		for (uint x = 0; x < mtxWidth; ++x) {
			if (locMtx[y][x] == maxColor)
				disMtx[y][x] = 0xFF;
			else {
				disMtx[y][x] = 0;
			}
		}
	}

	// 两次扫描计算距离矩阵和分配矩阵；
	// 第一次扫描（左下->右上）
	for (uint y = 0; y < bmpHeight; ++y) {
		for (uint x = 0; x < mtxWidth; ++x) {
			// 遍历所有矩阵位置（距离矩阵值为0跳过）
			if (disMtx[y][x] == 0)
				continue;
			uchar minDis = disMtx[y][x];

			//对任一（x, y）位置，遍历每一个模板元素
			for (uint i = 0, end = pDisT->getSize() / 2; i < end; ++i) {
				// 取出当前模板元素的参数（tmpDIS，OFFX，Y）
				int offX = pDisT->getOffX(i);
				int offY = pDisT->getOffY(i);
				float tmpDis = pDisT->getDis(i);

				// 计算当前模板元素在大矩阵中的位置（CX，CY）
				int CX = x + offX;
				int CY = y + offY;

				// 位置合法性检验
				if (CX < 0 || CX > bmpWidth - 1)
					continue;
				if (CY < 0 || CY > bmpHeight - 1)
					continue;

				// 该位置如果是无穷大不可计算
				if (disMtx[CY][CX] == 0xff)
					continue;

				// 计算距离矩阵对应距离值+模板元素的距离值，比较最小值
				if ((disMtx[CY][CX] + tmpDis) < minDis) {
					// 把当前（x,y）位置的距离值更新为最小值；同时，分配矩阵的值更新为最小距离来源的分配场值
					minDis = disMtx[CY][CX] + tmpDis;
					disMtx[y][x] = minDis;
					locMtx[y][x] = locMtx[CY][CX];
				}
			} // end for i
		}	// end for x
	}// end for y

	// 第二次扫描（右上->左下）
	for (int y = bmpHeight - 1; y >= 0; --y) {
		for (int x = bmpWidth - 1; x >= 0; --x) {
			if (disMtx[y][x] == 0)
				continue;
			uchar minDis = disMtx[y][x];

			//对任一（x, y）位置，遍历每一个模板元素
			for (int i = pDisT->getSize() / 2 + 1, end = pDisT->getSize(); i < end; ++i) {
				// 取出当前模板元素的参数（tmpDIS，OFFX，Y）
				int offX = pDisT->getOffX(i);
				int offY = pDisT->getOffY(i);
				float tmpDis = pDisT->getDis(i);

				// 计算当前模板元素在大矩阵中的位置（CX，CY）
				int CX = x + offX;
				int CY = y + offY;

				// 位置合法性检验
				if (CX < 0 || CX > bmpWidth - 1)
					continue;
				if (CY < 0 || CY > bmpHeight - 1)
					continue;

				// 该位置如果是无穷大不可计算
				if (disMtx[CY][CX] == 0xff)
					continue;

				// 计算距离矩阵对应距离值+模板元素的距离值，比较最小值
				if ((disMtx[CY][CX] + tmpDis) < minDis) {
					// 把当前（x,y）位置的距离值更新为最小值；同时，分配矩阵的值更新为最小距离来源的分配场值
					minDis = disMtx[CY][CX] + tmpDis;
					disMtx[y][x] = minDis;
					locMtx[y][x] = locMtx[CY][CX];
				}
			} // end for i
		} // end for x
	} // end for y

	// 获取14的缓冲区
	for (int y = 0; y < bmpHeight; ++y) {
		for (int x = 0; x < mtxWidth; ++x) {
			if (disMtx[y][x] > 14)
				locMtx[y][x] = 0xff;
			else
				locMtx[y][x] = 0;
		}
	}

	// 写入到bmp文件
	// 距离场
	Bmp bmpDisDest(8, bmpWidth, bmpHeight);
	bmpDisDest.copyClrTabFrom(bmpSrc.clrTab, 1024);
	bmpDisDest.copyMtxFrom(disMtx, mtxWidth, bmpHeight);
	bmpDisDest.write(disBmpName);
	// 分配场
	Bmp bmpLocDest(8, bmpWidth, bmpHeight);
	bmpLocDest.copyClrTabFrom(bmpSrc.clrTab, 1024);
	bmpLocDest.copyMtxFrom(locMtx, mtxWidth, bmpHeight);
	bmpLocDest.write(locBmpName);

	// 清理工作
	Delete2D<uchar>(locMtx, bmpHeight);
	Delete2D<uchar>(disMtx, bmpHeight);

	return true;
}


bool DisTrans32bit(const char* srcBmpName, DistanceTemplate* pDisT, const char* disBmpName, const char* locBmpName, uchar maxClor/*= 0xFF*/)
{
	Bmp bmpSrc;
	bmpSrc.read(srcBmpName);

	if (bmpSrc.bitCount() != 8) {
		Log("Input bmp's biCount is not 8!");
		return false;
	}

	uint bmpWidth = bmpSrc.width();
	uint bmpHeight = bmpSrc.height();
	uint mtxWidth = bmpSrc.mtxWidth;
	uint mtxHeight = bmpSrc.mtxHeight;

	// 分配场
	uchar** locMtx = nullptr;
	bmpSrc.copyMtxTo(&locMtx, nullptr, nullptr);

	// 距离场
	float** disMtx = New2D<float>(bmpHeight, bmpWidth);

	// 根据分配场的值初始化距离矩阵
	for (uint y = 0; y < bmpHeight; ++y) {
		for (uint x = 0; x < bmpWidth; ++x) {
			if (locMtx[y][x] == maxClor)
				disMtx[y][x] = MaxFloat;
			else
				disMtx[y][x] = 0.0f;
		}
	}

	// 两次扫描计算距离矩阵和分配矩阵；
	// 第一次扫描（左下->右上）
	for (uint y = 0; y < bmpHeight; ++y) {
		for (uint x = 0; x < bmpWidth; ++x) {
			// 遍历所有矩阵位置（距离矩阵值为0跳过）
			if (IsZero(disMtx[y][x]))
				continue;

			float minDis = disMtx[y][x];
			//对任一（x, y）位置，遍历每一个模板元素
			for (uint i = 0, end = pDisT->getSize() / 2 + 1; i < end; ++i) {
				// 取出当前模板元素的参数（tmpDIS，OFFX，Y）
				int offX = pDisT->getOffX(i);
				int offY = pDisT->getOffY(i);
				float tmpDis = pDisT->getDis(i);

				// 计算当前模板元素在大矩阵中的位置（CX，CY）
				int CX = x + offX;
				int CY = y + offY;

				// 位置合法性检验
				if (CX < 0 || CX > bmpWidth - 1 || CY < 0 || CY > bmpHeight - 1)
					continue;

				// 该位置如果是无穷大不可计算
				if (IsEqual(disMtx[CY][CX], MaxFloat))
					continue;

				// 计算距离矩阵对应距离值+模板元素的距离值，比较最小值
				if ((disMtx[CY][CX] + tmpDis) < minDis) {
					// 把当前（x,y）位置的距离值更新为最小值；同时，分配矩阵的值更新为最小距离来源的分配场值
					minDis = disMtx[CY][CX] + tmpDis;
					disMtx[y][x] = minDis;
					locMtx[y][x] = locMtx[CY][CX];
				}
			} // end for i
		}	// end for x
	}// end for y

	// 第二次扫描（右上->左下）
	// y 必须是 int，不能是 unsigned int
	for (int y = bmpHeight - 1; y >= 0; --y) {
		for (int x = bmpWidth - 1; x >= 0; --x) {
			if (IsZero(disMtx[y][x]))
				continue;

			float minDis = disMtx[y][x];
			//对任一（x, y）位置，遍历每一个模板元素
			for (int i = pDisT->getSize() / 2 + 1, end = pDisT->getSize(); i < end; ++i) {
				// 取出当前模板元素的参数（tmpDIS，OFFX，Y）
				int offX = pDisT->getOffX(i);
				int offY = pDisT->getOffY(i);
				float tmpDis = pDisT->getDis(i);

				// 计算当前模板元素在大矩阵中的位置（CX，CY）
				int CX = x + offX;
				int CY = y + offY;

				// 位置合法性检验
				if (CX < 0 || CX > bmpWidth - 1 || CY < 0 || CY > bmpHeight - 1)
					continue;

				// 该位置如果是无穷大不可计算
				if (IsEqual(disMtx[CY][CX], MaxFloat))
					continue;

				// 计算距离矩阵对应距离值+模板元素的距离值，比较最小值
				if ((disMtx[CY][CX] + tmpDis) < minDis) {
					// 把当前（x,y）位置的距离值更新为最小值；同时，分配矩阵的值更新为最小距离来源的分配场值
					minDis = disMtx[CY][CX] + tmpDis;
					disMtx[y][x] = minDis;
					locMtx[y][x] = locMtx[CY][CX];
				}
			} // end for i
		} // end for x
	} // end for y

	// 写入结果文件
	// 写入距离矩阵32位float型
	Bmp bmpDisDest(32, bmpWidth, bmpHeight);
	bmpDisDest.copyMtxFrom((uchar**)disMtx, bmpWidth * 4, bmpHeight);
	bmpDisDest.write(disBmpName);
	// 写入分配场矩阵8位uint型
	Bmp bmpLocDest(8, bmpWidth, bmpHeight);
	bmpLocDest.copyClrTabFrom(bmpSrc.clrTab, 1024);
	bmpLocDest.copyMtxFrom(locMtx, mtxWidth, mtxHeight);
	bmpLocDest.write(locBmpName);

	// 清理工作
	Delete2D<uchar>(locMtx, bmpHeight);
	Delete2D<float>(disMtx, bmpHeight);

	return true;
}

// 提取边界
bool GetBoundaryFromBmp(const char* bmpInputName, const char* bmpOutputName)
{
	// 读取文件
	Bmp bmpSrc;
	if (!bmpSrc.read(bmpInputName)) {
		LogF("Read %s Error", bmpInputName);
		return false;
	}

	uint bmpWidth = bmpSrc.width();
	uint bmpHeight = bmpSrc.height();
	uint mtxWidth = bmpSrc.mtxWidth;
	uint mtxHeight = bmpSrc.mtxHeight;

	// 提取边界，保存到boundaryMtx矩阵中
	uchar** boundaryMtx = nullptr;
	if (!GetBoundary(bmpSrc.mtx, mtxWidth, mtxHeight, &boundaryMtx)) {
		Log("Get boundary error");
		return false;
	}

	// 写入文件
	Bmp bmpBoundary(8, bmpWidth, bmpHeight);
	bmpBoundary.copyClrTabFrom(bmpSrc.clrTab, 1024);
	bmpBoundary.copyMtxFrom(boundaryMtx, mtxWidth, mtxHeight);
	bmpBoundary.write(bmpOutputName);
	
	Delete2D<uchar>(boundaryMtx, bmpHeight);
	return true;
}


bool GetBoundary(const uchar* const* srcMtx, uint mtxWidth, uint mtxHeight, uchar*** boundaryMtx)
{
	if (!boundaryMtx || *boundaryMtx) {
		Log("Input boundaryMtx error");
		return false;
	}

	*boundaryMtx = New2D<uchar>(mtxHeight, mtxWidth);
	
	for (int y = 1; y < mtxHeight - 1; ++y) {
		for (int x = 1; x < mtxWidth - 1; ++x) {
			uchar currClr  = srcMtx[y][x];
			uchar upClr	   = srcMtx[y + 1][x];
			uchar downClr  = srcMtx[y - 1][x];
			uchar leftClr  = srcMtx[y][x - 1];
			uchar rightClr = srcMtx[y][x + 1];

			if (currClr != upClr || currClr != downClr || currClr != leftClr || currClr != rightClr)
				(*boundaryMtx)[y][x] = 0x00;
			else
				(*boundaryMtx)[y][x] = 0xFF;
		}
	}
	
	return true;
}

// 从距离场提取缓冲区
// 输入bmp文件必须是32bit的
bool GetBufferFromBmp(const char* disBmpName, const char* locBmpName, const char* bmpOutputBufName, float radius)
{
	Bmp bmpSrc;
	bmpSrc.read(disBmpName);

	if (bmpSrc.bitCount() != 32) {
		Log("Input bmp's biCount is not 32!")
		return false;
	}

	uint bmpWidth = bmpSrc.width();
	uint bmpHeight = bmpSrc.height();
	uint mtxWidth = bmpSrc.mtxWidth;
	uint mtxHeight = bmpSrc.mtxHeight;

	uchar** bufMtx = nullptr;
	if (!GetBuffer((float**)bmpSrc.mtx, mtxWidth / 4, mtxHeight, &bufMtx, radius)) {
		Log("Get buffer error");
		return false;
	}

	Bmp bmpLoc;
	bmpLoc.read(locBmpName);

	Bmp bmpBuf(8, bmpWidth, bmpHeight);
	bmpBuf.fileHeader = bmpLoc.fileHeader;
	bmpBuf.infoHeader = bmpLoc.infoHeader;
	bmpBuf.copyClrTabFrom(bmpLoc.clrTab, 1024);
	bmpBuf.copyMtxFrom(bufMtx, mtxWidth / 4, mtxHeight);
	bmpBuf.write(bmpOutputBufName);

	Delete2D<uchar>(bufMtx, mtxHeight);
	return true;
}

// 获取缓冲区
// 输入：disMtx, radius
// 输出：bufferMtx
bool GetBuffer(const float* const* disMtx, uint mtxWidth, uint mtxHeight, uchar*** bufferMtx, float radius)
{
	if (!bufferMtx || *bufferMtx) {
		Log("Input bufferMtx error");
		return false;
	}

	// 分离缓冲区区域
	uchar** disMtxTemp = New2D<uchar>(mtxHeight, mtxWidth);
	for (uint y = 0; y < mtxHeight; ++y) {
		for (uint x = 0; x < mtxWidth; ++x) {
			if (disMtx[y][x] > radius)
				disMtxTemp[y][x] = 0xFF;
			else
				disMtxTemp[y][x] = disMtx[y][x];
		}
	}

	// 提取边界(即缓冲区边界)
	*bufferMtx = New2D<uchar>(mtxHeight, mtxWidth);
	for (int y = 1; y < mtxHeight - 1; ++y) {
		for (int x = 1; x < mtxWidth - 1; ++x) {
			uchar currClr  = disMtxTemp[y][x];
			uchar upClr	   = disMtxTemp[y + 1][x];
			uchar downClr  = disMtxTemp[y - 1][x];
			uchar leftClr  = disMtxTemp[y][x - 1];
			uchar rightClr = disMtxTemp[y][x + 1];

			//if (currClr != upClr || currClr != downClr || currClr != leftClr || currClr != rightClr)
				(*bufferMtx)[y][x] = disMtxTemp[y][x];
			//else
			//	(*bufferMtx)[y][x] = 255;
		}
	}
	
	Delete2D<uchar>(disMtxTemp, mtxHeight);
	return true;
}
