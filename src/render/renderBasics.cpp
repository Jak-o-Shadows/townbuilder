//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#include "renderBasics.hpp"

#include <cmath>

#include <string.h>
#include <iostream>

namespace Render{
namespace Basics{

#define POINTSIZE 5

duDebugDraw::~duDebugDraw()
{
	// Empty
}

ImVec4 duDebugDraw::areaToCol(unsigned int area)
{
	if (area == 0)
	{
		// Treat zero area type as default.
		return duRGBA(0, 192, 255, 255);
	}
	else
	{
		return duIntToCol(area, 255);
	}
}

inline int bit(int a, int b)
{
	return (a & (1 << b)) >> b;
}

ImVec4 duIntToCol(int i, int a)
{
	int	r = bit(i, 1) + bit(i, 3) * 2 + 1;
	int	g = bit(i, 2) + bit(i, 4) * 2 + 1;
	int	b = bit(i, 0) + bit(i, 5) * 2 + 1;
	return duRGBA(r*63,g*63,b*63,a);
}

void duIntToCol(int i, ImVec4* col)
{
	int	r = bit(i, 0) + bit(i, 3) * 2 + 1;
	int	g = bit(i, 1) + bit(i, 4) * 2 + 1;
	int	b = bit(i, 2) + bit(i, 5) * 2 + 1;
	col->x = 1 - r*63.0f/255.0f;
	col->y = 1 - g*63.0f/255.0f;
	col->z = 1 - b*63.0f/255.0f;
}

void duCalcBoxColors(ImVec4* colors, ImVec4 colTop, ImVec4 colSide)
{
	if (!colors) return;
	
	colors[0] = duMultCol(colTop, 250);
	colors[1] = duMultCol(colSide, 140);
	colors[2] = duMultCol(colSide, 165);
	colors[3] = duMultCol(colSide, 217);
	colors[4] = duMultCol(colSide, 165);
	colors[5] = duMultCol(colSide, 217);
}

void duDebugDrawCylinderWire(ImDrawList* dd, float minx, float miny, float minz,
							 float maxx, float maxy, float maxz, ImVec4 col, const float lineWidth)
{
	if (!dd) return;
	
	//dd->begin(DU_DRAW_LINES, lineWidth);
	duAppendCylinderWire(dd, minx,miny,minz, maxx,maxy,maxz, col);
	//dd->end();
}

void duDebugDrawBoxWire(ImDrawList* dd, float minx, float miny, float minz,
						float maxx, float maxy, float maxz, ImVec4 col, const float lineWidth)
{
	if (!dd) return;
	
	//dd->begin(DU_DRAW_LINES, lineWidth);
	duAppendBoxWire(dd, minx,miny,minz, maxx,maxy,maxz, col);
	//dd->end();
}

void duDebugDrawArc(ImDrawList* dd, const float x0, const float y0, const float z0,
					const float x1, const float y1, const float z1, const float h,
					const float as0, const float as1, ImVec4 col, const float lineWidth)
{
	if (!dd) return;
	
	//dd->begin(DU_DRAW_LINES, lineWidth);
	duAppendArc(dd, x0,y0,z0, x1,y1,z1, h, as0, as1, col);
	//dd->end();
}

void duDebugDrawArrow(ImDrawList* dd, const float x0, const float y0, const float z0,
					  const float x1, const float y1, const float z1,
					  const float as0, const float as1, ImVec4 col, const float lineWidth)
{
	if (!dd) return;
	
	//dd->begin(DU_DRAW_LINES, lineWidth);
	duAppendArrow(dd, x0,y0,z0, x1,y1,z1, as0, as1, col);
	//dd->end();
}

void duDebugDrawCircle(ImDrawList* dd, const float x, const float y, const float z,
					   const float r, ImVec4 col, const float lineWidth)
{
	if (!dd) return;
	
	//dd->begin(DU_DRAW_LINES, lineWidth);
	duAppendCircle(dd, x,y,z, r, col);
	//dd->end();
}

void duDebugDrawCross(ImDrawList* dd, const float x, const float y, const float z,
					  const float size, ImVec4 col, const float lineWidth)
{
	if (!dd) return;
	
	//dd->begin(DU_DRAW_LINES, lineWidth);
	duAppendCross(dd, x,y,z, size, col);
	//dd->end();
}

void duDebugDrawBox(ImDrawList* dd, float minx, float miny, float minz,
					float maxx, float maxy, float maxz, const ImVec4* fcol)
{
	if (!dd) return;
	
	//dd->begin(DU_DRAW_QUADS);
	duAppendBox(dd, minx,miny,minz, maxx,maxy,maxz, fcol);
	//dd->end();
}

void duDebugDrawCylinder(ImDrawList* dd, float minx, float miny, float minz,
						 float maxx, float maxy, float maxz, ImVec4 col)
{
	if (!dd) return;
	
	//dd->begin(DU_DRAW_TRIS);
	duAppendCylinder(dd, minx,miny,minz, maxx,maxy,maxz, col);
	//dd->end();
}

void duDebugDrawGridXZ(ImDrawList* dd, const float ox, const float oy, const float oz,
					   const int w, const int h, const float size,
					   const ImVec4 col, const float lineWidth)
{
	if (!dd) return;

	//dd->begin(DU_DRAW_LINES, lineWidth);
	for (int i = 0; i <= h; ++i)
	{
		dd->AddCircleFilled(ImVec3(ox,oy,oz+i*size), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(ox+w*size,oy,oz+i*size), POINTSIZE, ImColor(col));
	}
	for (int i = 0; i <= w; ++i)
	{
		dd->AddCircleFilled(ImVec3(ox+i*size,oy,oz), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(ox+i*size,oy,oz+h*size), POINTSIZE, ImColor(col));
	}
	//dd->end();
}
		 

void duAppendCylinderWire(ImDrawList* dd, float minx, float miny, float minz,
						  float maxx, float maxy, float maxz, ImVec4 col)
{
	if (!dd) return;

	static const int NUM_SEG = 16;
	static float dir[NUM_SEG*2];
	static bool init = false;
	if (!init)
	{
		init = true;
		for (int i = 0; i < NUM_SEG; ++i)
		{
			const float a = (float)i/(float)NUM_SEG*DU_PI*2;
			dir[i*2] = std::cos(a);
			dir[i*2+1] = std::sin(a);
		}
	}
	
	const float cx = (maxx + minx)/2;
	const float cz = (maxz + minz)/2;
	const float rx = (maxx - minx)/2;
	const float rz = (maxz - minz)/2;
	
	for (int i = 0, j = NUM_SEG-1; i < NUM_SEG; j = i++)
	{
		dd->AddCircleFilled(ImVec3(cx+dir[j*2+0]*rx, miny, cz+dir[j*2+1]*rz), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(cx+dir[i*2+0]*rx, miny, cz+dir[i*2+1]*rz), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(cx+dir[j*2+0]*rx, maxy, cz+dir[j*2+1]*rz), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(cx+dir[i*2+0]*rx, maxy, cz+dir[i*2+1]*rz), POINTSIZE, ImColor(col));
	}
	for (int i = 0; i < NUM_SEG; i += NUM_SEG/4)
	{
		dd->AddCircleFilled(ImVec3(cx+dir[i*2+0]*rx, miny, cz+dir[i*2+1]*rz), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(cx+dir[i*2+0]*rx, maxy, cz+dir[i*2+1]*rz), POINTSIZE, ImColor(col));
	}
}

void duAppendBoxWire(ImDrawList* dd, float minx, float miny, float minz,
					 float maxx, float maxy, float maxz, ImVec4 col)
{
	if (!dd) return;
	// Top
	dd->AddCircleFilled(ImVec3(minx, miny, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, miny, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, miny, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, miny, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, miny, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, miny, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, miny, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, miny, minz), POINTSIZE, ImColor(col));
	
	// bottom
	dd->AddCircleFilled(ImVec3(minx, maxy, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, maxy, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, maxy, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, maxy, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, maxy, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, maxy, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, maxy, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, maxy, minz), POINTSIZE, ImColor(col));
	
	// Sides
	dd->AddCircleFilled(ImVec3(minx, miny, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, maxy, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, miny, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, maxy, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, miny, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, maxy, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, miny, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, maxy, maxz), POINTSIZE, ImColor(col));
}

void duAppendBoxPoints(ImDrawList* dd, float minx, float miny, float minz,
					   float maxx, float maxy, float maxz, ImVec4 col)
{
	if (!dd) return;
	// Top
	dd->AddCircleFilled(ImVec3(minx, miny, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, miny, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, miny, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, miny, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, miny, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, miny, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, miny, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, miny, minz), POINTSIZE, ImColor(col));
	
	// bottom
	dd->AddCircleFilled(ImVec3(minx, maxy, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, maxy, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, maxy, minz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, maxy, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(maxx, maxy, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, maxy, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, maxy, maxz), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(minx, maxy, minz), POINTSIZE, ImColor(col));
}

void duAppendBox(ImDrawList* dd, float minx, float miny, float minz,
				 float maxx, float maxy, float maxz, const ImVec4* fcol)
{
	if (!dd) return;

	// Note: Only doing the 2D projection of the cuboid
	ImVec2 top_left(minx, miny);
	ImVec2 top_right(maxx, miny);
	ImVec2 bottom_right(maxx, maxy);
	ImVec2 bottom_left(minx, maxy);

	// Thickness of the lines
	float thickness = 2.0f;

	ImColor color = ImColor(fcol[0]);

	// Draw the outline of the quadrilateral
	dd->AddLine(top_left, top_right, color, thickness);
	dd->AddLine(top_right, bottom_right, color, thickness);
	dd->AddLine(bottom_right, bottom_left, color, thickness);
	dd->AddLine(bottom_left, top_left, color, thickness);

	// If you want to fill the quadrilateral, use AddConvexPolyFilled:
	ImVec2 points[] = {top_left, top_right, bottom_right, bottom_left};
	dd->AddConvexPolyFilled(points, 4, color);

}

void duAppendCylinder(ImDrawList* dd, float minx, float miny, float minz,
					  float maxx, float maxy, float maxz, ImVec4 col)
{
	if (!dd) return;
	
	static const int NUM_SEG = 16;
	static float dir[NUM_SEG*2];
	static bool init = false;
	if (!init)
	{
		init = true;
		for (int i = 0; i < NUM_SEG; ++i)
		{
			const float a = (float)i/(float)NUM_SEG*DU_PI*2;
			dir[i*2] = cosf(a);
			dir[i*2+1] = sinf(a);
		}
	}
	
	ImVec4 col2 = duMultCol(col, 160);
	
	const float cx = (maxx + minx)/2;
	const float cz = (maxz + minz)/2;
	const float rx = (maxx - minx)/2;
	const float rz = (maxz - minz)/2;

	for (int i = 2; i < NUM_SEG; ++i)
	{
		const int a = 0, b = i-1, c = i;
		dd->AddCircleFilled(ImVec3(cx+dir[a*2+0]*rx, miny, cz+dir[a*2+1]*rz), POINTSIZE, ImColor(col2));
		dd->AddCircleFilled(ImVec3(cx+dir[b*2+0]*rx, miny, cz+dir[b*2+1]*rz), POINTSIZE, ImColor(col2));
		dd->AddCircleFilled(ImVec3(cx+dir[c*2+0]*rx, miny, cz+dir[c*2+1]*rz), POINTSIZE, ImColor(col2));
	}
	for (int i = 2; i < NUM_SEG; ++i)
	{
		const int a = 0, b = i, c = i-1;
		dd->AddCircleFilled(ImVec3(cx+dir[a*2+0]*rx, maxy, cz+dir[a*2+1]*rz), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(cx+dir[b*2+0]*rx, maxy, cz+dir[b*2+1]*rz), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(cx+dir[c*2+0]*rx, maxy, cz+dir[c*2+1]*rz), POINTSIZE, ImColor(col));
	}
	for (int i = 0, j = NUM_SEG-1; i < NUM_SEG; j = i++)
	{
		dd->AddCircleFilled(ImVec3(cx+dir[i*2+0]*rx, miny, cz+dir[i*2+1]*rz), POINTSIZE, ImColor(col2));
		dd->AddCircleFilled(ImVec3(cx+dir[j*2+0]*rx, miny, cz+dir[j*2+1]*rz), POINTSIZE, ImColor(col2));
		dd->AddCircleFilled(ImVec3(cx+dir[j*2+0]*rx, maxy, cz+dir[j*2+1]*rz), POINTSIZE, ImColor(col));

		dd->AddCircleFilled(ImVec3(cx+dir[i*2+0]*rx, miny, cz+dir[i*2+1]*rz), POINTSIZE, ImColor(col2));
		dd->AddCircleFilled(ImVec3(cx+dir[j*2+0]*rx, maxy, cz+dir[j*2+1]*rz), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(cx+dir[i*2+0]*rx, maxy, cz+dir[i*2+1]*rz), POINTSIZE, ImColor(col));
	}
}


inline void evalArc(const float x0, const float y0, const float z0,
					const float dx, const float dy, const float dz,
					const float h, const float u, float* res)
{
	res[0] = x0 + dx * u;
	res[1] = y0 + dy * u + h * (1-(u*2-1)*(u*2-1));
	res[2] = z0 + dz * u;
}


inline void vcross(float* dest, const float* v1, const float* v2)
{
	dest[0] = v1[1]*v2[2] - v1[2]*v2[1];
	dest[1] = v1[2]*v2[0] - v1[0]*v2[2];
	dest[2] = v1[0]*v2[1] - v1[1]*v2[0]; 
}

inline void vnormalize(float* v)
{
	float d = 1.0f / sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	v[0] *= d;
	v[1] *= d;
	v[2] *= d;
}

inline void vsub(float* dest, const float* v1, const float* v2)
{
	dest[0] = v1[0]-v2[0];
	dest[1] = v1[1]-v2[1];
	dest[2] = v1[2]-v2[2];
}

inline float vdistSqr(const float* v1, const float* v2)
{
	const float x = v1[0]-v2[0];
	const float y = v1[1]-v2[1];
	const float z = v1[2]-v2[2];
	return x*x + y*y + z*z;
}


void appendArrowHead(ImDrawList* dd, const float* p, const float* q,
					 const float s, ImVec4 col)
{
	const float eps = 0.001f;
	if (!dd) return;
	if (vdistSqr(p,q) < eps*eps) return;
	float ax[3], ay[3] = {0,1,0}, az[3];
	vsub(az, q, p);
	vnormalize(az);
	vcross(ax, ay, az);
	vcross(ay, az, ax);
	vnormalize(ay);

	dd->AddCircleFilled(ImVec3(p), POINTSIZE, ImColor(col));
//	dd->AddCircleFilled(ImVec3(p[0]+az[0]*s+ay[0]*s/2, p[1]+az[1]*s+ay[1]*s/2, p[2]+az[2]*s+ay[2]*s/2, ImColor(col));
	dd->AddCircleFilled(ImVec3(p[0]+az[0]*s+ax[0]*s/3, p[1]+az[1]*s+ax[1]*s/3, p[2]+az[2]*s+ax[2]*s/3), POINTSIZE, ImColor(col));

	dd->AddCircleFilled(ImVec3(p), POINTSIZE, ImColor(col));
//	dd->AddCircleFilled(ImVec3(p[0]+az[0]*s-ay[0]*s/2, p[1]+az[1]*s-ay[1]*s/2, p[2]+az[2]*s-ay[2]*s/2, ImColor(col));
	dd->AddCircleFilled(ImVec3(p[0]+az[0]*s-ax[0]*s/3, p[1]+az[1]*s-ax[1]*s/3, p[2]+az[2]*s-ax[2]*s/3), POINTSIZE, ImColor(col));
	
}

void duAppendArc(ImDrawList* dd, const float x0, const float y0, const float z0,
				 const float x1, const float y1, const float z1, const float h,
				 const float as0, const float as1, ImVec4 col)
{
	if (!dd) return;
	static const int NUM_ARC_PTS = 8;
	static const float PAD = 0.05f;
	static const float ARC_PTS_SCALE = (1.0f-PAD*2) / (float)NUM_ARC_PTS;
	const float dx = x1 - x0;
	const float dy = y1 - y0;
	const float dz = z1 - z0;
	const float len = sqrtf(dx*dx + dy*dy + dz*dz);
	float prev[3];
	evalArc(x0,y0,z0, dx,dy,dz, len*h, PAD, prev);
	for (int i = 1; i <= NUM_ARC_PTS; ++i)
	{
		const float u = PAD + i * ARC_PTS_SCALE;
		float pt[3];
		evalArc(x0,y0,z0, dx,dy,dz, len*h, u, pt);
		dd->AddCircleFilled(ImVec3(prev[0],prev[1],prev[2]), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(pt[0],pt[1],pt[2]), POINTSIZE, ImColor(col));
		prev[0] = pt[0]; prev[1] = pt[1]; prev[2] = pt[2];
	}
	
	// End arrows
	if (as0 > 0.001f)
	{
		float p[3], q[3];
		evalArc(x0,y0,z0, dx,dy,dz, len*h, PAD, p);
		evalArc(x0,y0,z0, dx,dy,dz, len*h, PAD+0.05f, q);
		appendArrowHead(dd, p, q, as0, col);
	}

	if (as1 > 0.001f)
	{
		float p[3], q[3];
		evalArc(x0,y0,z0, dx,dy,dz, len*h, 1-PAD, p);
		evalArc(x0,y0,z0, dx,dy,dz, len*h, 1-(PAD+0.05f), q);
		appendArrowHead(dd, p, q, as1, col);
	}
}

void duAppendArrow(ImDrawList* dd, const float x0, const float y0, const float z0,
				   const float x1, const float y1, const float z1,
				   const float as0, const float as1, ImVec4 col)
{
	if (!dd) return;

	dd->AddCircleFilled(ImVec3(x0,y0,z0), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(x1,y1,z1), POINTSIZE, ImColor(col));
	
	// End arrows
	const float p[3] = {x0,y0,z0}, q[3] = {x1,y1,z1};
	if (as0 > 0.001f)
		appendArrowHead(dd, p, q, as0, col);
	if (as1 > 0.001f)
		appendArrowHead(dd, q, p, as1, col);
}

void duAppendCircle(ImDrawList* dd, const float x, const float y, const float z,
					const float r, ImVec4 col)
{
	if (!dd) return;
	static const int NUM_SEG = 40;
	static float dir[40*2];
	static bool init = false;
	if (!init)
	{
		init = true;
		for (int i = 0; i < NUM_SEG; ++i)
		{
			const float a = (float)i/(float)NUM_SEG*DU_PI*2;
			dir[i*2] = std::cos(a);
			dir[i*2+1] = std::sin(a);
		}
	}
	
	for (int i = 0, j = NUM_SEG-1; i < NUM_SEG; j = i++)
	{
		dd->AddCircleFilled(ImVec3(x+dir[j*2+0]*r, y, z+dir[j*2+1]*r), POINTSIZE, ImColor(col));
		dd->AddCircleFilled(ImVec3(x+dir[i*2+0]*r, y, z+dir[i*2+1]*r), POINTSIZE, ImColor(col));
	}
}

void duAppendCross(ImDrawList* dd, const float x, const float y, const float z,
				   const float s, ImVec4 col)
{
	if (!dd) return;
	dd->AddCircleFilled(ImVec3(x-s,y,z), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(x+s,y,z), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(x,y-s,z), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(x,y+s,z), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(x,y,z-s), POINTSIZE, ImColor(col));
	dd->AddCircleFilled(ImVec3(x,y,z+s), POINTSIZE, ImColor(col));
}

duDisplayList::duDisplayList(int cap) :
	m_pos(0),
	m_color(0),
	m_size(0),
	m_cap(0),
	m_prim(DU_DRAW_LINES),
	m_primSize(1.0f),
	m_depthMask(true)
{
	if (cap < 8)
		cap = 8;
	resize(cap);
}

duDisplayList::~duDisplayList()
{
	delete [] m_pos;
	delete [] m_color;
}

void duDisplayList::resize(int cap)
{
	float* newPos = new float[cap*3];
	if (m_size)
		memcpy(newPos, m_pos, sizeof(float)*3*m_size);
	delete [] m_pos;
	m_pos = newPos;

	ImVec4* newColor = new ImVec4[cap];
	if (m_size)
		memcpy(newColor, m_color, sizeof(unsigned int)*m_size);
	delete [] m_color;
	m_color = newColor;
	
	m_cap = cap;
}

void duDisplayList::clear()
{
	m_size = 0;
}

void duDisplayList::depthMask(bool state)
{
	m_depthMask = state;
}

void duDisplayList::begin(duDebugDrawPrimitives prim, float size)
{
	clear();
	m_prim = prim;
	m_primSize = size;
}

void duDisplayList::vertex(const float x, const float y, const float z, ImVec4 color)
{
	if (m_size+1 >= m_cap)
		resize(m_cap*2);
	float* p = &m_pos[m_size*3];
	p[0] = x;
	p[1] = y;
	p[2] = z;
	m_color[m_size] = color;
	m_size++;
}

void duDisplayList::vertex(const float* pos, ImVec4 color)
{
	vertex(pos[0],pos[1],pos[2],color);
}

void duDisplayList::end()
{
}

void duDisplayList::draw(ImDrawList* dd)
{
	if (!dd) return;
	if (!m_size) return;
	//dd->depthMask(m_depthMask);
	//dd->begin(m_prim, m_primSize);
	for (int i = 0; i < m_size; ++i)
		dd->AddCircleFilled(ImVec3(&m_pos[i*3]), POINTSIZE, ImColor(m_color[i]));
	//dd->end();
}

}
}