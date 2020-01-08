/**************************************************************************
**
** description: 粘连变换、提取复杂多边形中轴线、构造Delaunay三角网
**
** author:  @github: Leopard-C/MapAlgebra
**
** last change: 2020-01-08
**
**************************************************************************/
#pragma once

#include "dis_template.hpp"


// 粘连变换
bool AdhesionTransform(const char* srcBmpName, DistanceTemplate* tmpT, float outRadius, float inRadius, const char* destBmpName);

// 提取中轴
bool GetAxis(const char* srcBmpName, DistanceTemplate* tmpT, const char* destBmpName);

// Delauney三角网
bool ScanSrcPointCoords(const char* srcBmpName, const char* coordsTableFile);
bool GetTinPointPairs(const char* srcLocBmpName, const char* pointPairsFile);
bool LinkPoints(const char* srcBmpName, const char* pointPairsFile, const char* coordsTableFile, const char* outBmpFile);

