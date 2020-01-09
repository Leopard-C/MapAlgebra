#include "MA3.h"
#include "MA2.h"
#include "MA1.h"
#include "bmp.h"

#include <cstdio>
#include <set>

#pragma warning(disable : 4996)


// 读取点坐标到文件
bool ScanSrcPointCoords(const char* srcBmpName, const char* coordsTableFile)
{
	Bmp bmpSrc;
	if (!bmpSrc.read(srcBmpName)) {
		LogF("Read %s error", srcBmpName);
		return false;
	}

	FILE* fp = fopen(coordsTableFile, "w+");
	if (!fp) {
		Log("Open coordsTableFile error");
		return false;
	}

	uint bmpWidth = bmpSrc.width();
	uint bmpHeight = bmpSrc.height();
	
	for (uint y = 0; y < bmpHeight; ++y) {
		for (uint x = 0; x < bmpWidth; ++x) {
			if (bmpSrc.mtx[y][x] != 0xff) {
				fprintf(fp, "%d\t%d\t%d\n", bmpSrc.mtx[y][x], x, y);
			}
		}
	}

	fclose(fp);
	return true;
}

// 扫描分配场，找出Voronoi相邻关系点对
bool GetTinPointPairs(const char* srcLocBmpName, const char* pointPairsFile)
{
	Bmp bmpLoc;
	if (!bmpLoc.read(srcLocBmpName)) {
		Log("Read locBmpFile error");
		return false;
	}

	uint bmpWidth = bmpLoc.width();
	uint bmpHeight = bmpLoc.height();
	uint mtxWidth = bmpLoc.mtxWidth;
	uint mtxHeight = bmpLoc.mtxHeight;
	const uchar* const* locMtx = bmpLoc.mtx;

	// 扫描分配场，获取点对信息存入点对数组
	uchar clr1[500];
	std::set<std::pair<uchar, uchar>> clrSet;	// 使用std::set容器，自动排序，且元素是唯一的
	int ptsPairNum = 0;

	for (uint y = 1; y < bmpHeight - 1; ++y) {
		for (uint x = 1; x < bmpWidth - 1; ++x) {
			uchar currClr = locMtx[y][x];
			uchar upClr    = locMtx[y + 1][x];
			uchar downClr  = locMtx[y - 1][x];
			uchar leftClr  = locMtx[y][x - 1];
			uchar rightClr = locMtx[y][x + 1];

			// 找出四邻域中的异色
			uchar tmpClr;
			if (currClr != upClr)
				tmpClr = upClr;
			else if (currClr != downClr)
				tmpClr = downClr;
			else if (currClr != leftClr)
				tmpClr = leftClr;
			else if (currClr != rightClr)
				tmpClr = rightClr;
			else
				continue;

			uchar minClr = currClr;
			uchar maxClr = tmpClr;
			if (currClr > tmpClr) {
				maxClr = currClr;
				minClr = tmpClr;
			}

			clrSet.insert({ minClr, maxClr });
		}
	}

	FILE* fp = fopen(pointPairsFile, "w+");
	if (!fp) {
		LogF("Open %s error", pointPairsFile);
		return false;
	}

	for (const auto& clrPair : clrSet) {
		fprintf(fp, "%d\t%d\n", clrPair.first, clrPair.second);
	}
	fclose(fp);

	return true;
}

// 连接点
bool LinkPoints(const char* srcBmpName, const char* pointPairsFile, const char* coordsTableFile, const char* outBmpFile)
{
	Bmp bmpSrc;
	if (!bmpSrc.read(srcBmpName)) {
		LogF("Read %s error", srcBmpName);
		return false;
	}

	uchar** srcMtx = bmpSrc.mtx;

	int tabX[256];
	int tabY[256];

	// 读取 coordsTableFile
	FILE* fp = fopen(coordsTableFile, "r");
	if (!fp) {
		LogF("Read %s error", coordsTableFile);
		return false;
	}
	while (!feof(fp)) {
		uint color;
		uint x = 0, y = 0;
		fscanf(fp, "%d\t%d\t%d\n", &color, &x, &y);
		tabX[color] = x;
		tabY[color] = y;
	}
	fclose(fp);

	// 读取 pointPairsFile
	// 同时使用DDA绘制两点之间的线段
	fp = fopen(pointPairsFile, "r");
	if (!fp) {
		LogF("Read %s error", pointPairsFile);
		return false;
	}
	while (!feof(fp)) {
		uint clr1 = 0, clr2 = 0;
		fscanf(fp, "%d\t%d\n", &clr1, &clr2);

		int xa = tabX[clr1];
		int ya = tabY[clr1];
		int xb = tabX[clr2];
		int yb = tabY[clr2];

		// DDA 绘制线段
		int dx = xb - xa;	// 两端点间的dx
		int dy = yb - ya;	// 两端点间的dy
		int n;		// 步数
		if (abs(dx) > abs(dy))
			n = abs(dx);
		else
			n = abs(dy);
		// x,y步进值
		float xinc = (float)dx / n;
		float yinc = (float)dy / n;
		// 当前点坐标
		float currX = xa, currY = ya;
		for (int k = 0; k < n; ++k) {
			srcMtx[int(currY + 0.5)][int(currX + 0.5)] = 0;
			currX += xinc;
			currY += yinc;
		}
	}
	fclose(fp);

	// 写入文件
	bmpSrc.write(outBmpFile);

	return true;
}

// 粘连变换
bool AdhesionTransform(const char* srcBmpName, DistanceTemplate* tmpT, float outRadius, float inRadius, const char* destBmpName)
{
	// 外距变换
	DisTrans32bit(srcBmpName, tmpT, "data/temp/dis.bmp", "data/temp/loc.bmp", 255);
	GetBufferFromBmp("data/temp/dis.bmp", "data/temp/loc.bmp", "data/temp/buffer.bmp", outRadius);

	// 内距变换
	DisTrans32bit("data/temp/buffer.bmp", tmpT, "data/temp/dis.bmp", "data/temp/loc.bmp", 0);
	GetBufferFromBmp("data/temp/dis.bmp", "data/temp/loc.bmp", destBmpName, inRadius);

	BmpInverse(destBmpName, destBmpName);

	return true;
}

// 提取中轴
bool GetAxis(const char* srcBmpName, DistanceTemplate* tmpT, const char* destBmpName)
{
	// 内距变换
	DisTrans32bit(srcBmpName, tmpT, "data/temp/axis_dis.bmp", "data/temp/axis_loc.bmp", 0);
	// 提取边界线
	GetBoundaryFromBmp("data/temp/axis_loc.bmp", destBmpName);
	return true;
}



