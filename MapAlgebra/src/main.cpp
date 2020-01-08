#include "MA1.h"
#include "MA2.h"
#include "MA3.h"
#include "dis_template.hpp"


int main()
{
	/* 1-1. 反色处理 */
	BmpInverse("data/input/8bit/shape.bmp", "data/output/shape_inverse_8bit.bmp");
	BmpInverse("data/input/24bit/landscape.bmp", "data/output/landscape_inverse_24bit.bmp");

	/* 1-2. 叠置处理 */
	BmpBooleanAnd("data/input/8bit/left.bmp", "data/input/8bit/right.bmp", "data/output/boolean_and.bmp");
	BmpBooleanOr("data/input/8bit/left.bmp", "data/input/8bit/right.bmp", "data/output/boolean_or.bmp");
	BmpBooleanXOr("data/input/8bit/left.bmp", "data/input/8bit/right.bmp", "data/output/boolean_xor.bmp");
	BmpBooleanNot("data/input/8bit/left.bmp", "data/output/boolean_not.bmp");

	/* 1-3. 平滑处理 */
	BmpSmooth("data/input/8bit/shape.bmp", "data/output/shape_smooth.bmp");

	/* 1-4. 8bit转32bit */
	Bmp256ColorTo32Bit("data/input/8bit/shape.bmp", "data/output/shape_32bit.bmp");

	/* 1-5. 24bit转8bit */
	Bmp24BitTo256Color("data/input/24bit/landscape.bmp", "data/output/landscape_8bit_from_24bit.bmp");

	/*************************************************************************************************************************************/


	/* 2-1. 距离变换 */
	OctagoTemplate octagoTem;
	DisTrans256("data/input/8bit/shape.bmp", &octagoTem, "data/output/shape_dis_256.bmp", "data/output/shape_loc.bmp");
	DisTrans32bit("data/input/8bit/shape.bmp", &octagoTem, "data/output/shape_dis_32bit.bmp", "data/output/shape_loc.bmp");

	/* 2-2. Voronoi图 */
	EurTemplate<13> eur13Tem;
	DisTrans32bit("data/input/8bit/points.bmp", &eur13Tem, "data/temp/dis.bmp", "data/output/voronoi.bmp");

	/* 2-3. 提取缓冲区 */
	DisTrans32bit("data/input/8bit/shape.bmp", &eur13Tem, "data/temp/dis.bmp", "data/temp/loc.bmp");
	GetBufferFromBmp("data/temp/dis.bmp", "data/temp/loc.bmp", "data/output/buffer.bmp", 20);

	/*************************************************************************************************************************************/


	/* 3-1. 粘连变换 */
	AdhesionTransform("data/input/8bit/shape.bmp", &eur13Tem, 20, 30, "data/output/adhesionTrans.bmp");

	/* 3-2. 提取中轴 */
	GetAxis("data/input/8bit/axis_test.bmp", &eur13Tem, "data/output/axis.bmp");

	/* 3-3. Delaunay三角网 */
	ScanSrcPointCoords("data/input/8bit/points.bmp", "data/temp/coords.txt");
	DisTrans32bit("data/input/8bit/points.bmp", &eur13Tem, "data/temp/dis.bmp", "data/temp/loc.bmp");
	GetTinPointPairs("data/temp/loc.bmp", "data/temp/PointPairs.txt");
	LinkPoints("data/input/8bit/points.bmp", "data/temp/PointPairs.txt",
		"data/temp/coords.txt", "data/output/TIN.bmp");

	return 0;
}

