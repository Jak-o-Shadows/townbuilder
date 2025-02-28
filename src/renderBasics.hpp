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
#pragma once

#include "imgui.h"

namespace Render {

namespace Basics {


struct ImVec3 : public ImVec2
{
	float z;

	ImVec3() : ImVec2(), z(0.0f) {}
	ImVec3(const float* p): ImVec2(p[0], p[1]), z(p[2]) {}
	ImVec3(float x, float y, float z) : ImVec2(x, y), z(z) {}
	ImVec3(const ImVec2& vec, float z = 0.0f) : ImVec2(vec), z(z) {}
};

// Some math headers don't have PI defined.
static const float DU_PI = 3.14159265f;

enum duDebugDrawPrimitives
{
	DU_DRAW_POINTS,
	DU_DRAW_LINES,
	DU_DRAW_TRIS,
	DU_DRAW_QUADS
};

/// Abstract debug draw interface.
struct duDebugDraw
{
	virtual ~duDebugDraw() = 0;
	
	virtual void depthMask(bool state) = 0;

	virtual void texture(bool state) = 0;

	/// Begin drawing primitives.
	///  @param prim [in] primitive type to draw, one of rcDebugDrawPrimitives.
	///  @param size [in] size of a primitive, applies to point size and line width only.
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f) = 0;

	/// Submit a vertex
	///  @param pos [in] position of the verts.
	///  @param color [in] color of the verts.
	virtual void vertex(const float* pos, ImVec4 color) = 0;

	/// Submit a vertex
	///  @param x,y,z [in] position of the verts.
	///  @param color [in] color of the verts.
	virtual void vertex(const float x, const float y, const float z, ImVec4 color) = 0;

	/// Submit a vertex
	///  @param pos [in] position of the verts.
	///  @param color [in] color of the verts.
	///  @param uv [in] the uv coordinates of the verts.
	virtual void vertex(const float* pos, ImVec4 color, const float* uv) = 0;
	
	/// Submit a vertex
	///  @param x,y,z [in] position of the verts.
	///  @param color [in] color of the verts.
	///  @param u,v [in] the uv coordinates of the verts.
	virtual void vertex(const float x, const float y, const float z, ImVec4 color, const float u, const float v) = 0;
	
	/// End drawing primitives.
	virtual void end() = 0;

	/// Compute a color for given area.
	virtual ImVec4 areaToCol(unsigned int area);
};

ImVec4 duRGBA(int r, int g, int b, int a)
{
    return ImVec4(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
}

ImVec4 duRGBAf(float fr, float fg, float fb, float fa)
{
	return ImVec4(fr, fg, fb, fa);
}

ImVec4 duIntToCol(int i, int a);
void duIntToCol(int i, ImVec4* col);

ImVec4 duMultCol(const ImVec4 col, const unsigned int d)
{
    return ImVec4(col.x*d, col.y*d, col.z*d, col.w);
}

ImVec4 duDarkenCol(const ImVec4 col)
{
    return col;  // TODO: Darken it somehow?
	//return ((col >> 1) & 0x007f7f7f) | (col & 0xff000000);
}

ImVec4 duLerpCol(const ImVec4 ca, const ImVec4 cb, const unsigned int u)
{

	unsigned int r = (255*ca.x*(255-u) + 255*cb.x*u)/255;
	unsigned int g = (255*ca.y*(255-u) + 255*cb.y*u)/255;
	unsigned int b = (255*ca.z*(255-u) + 255*cb.z*u)/255;
	unsigned int a = (255*ca.w*(255-u) + 255*cb.w*u)/255;
	return duRGBA(r,g,b,a);
}

ImVec4 duTransCol(const ImVec4 c, unsigned int a)
{
    return ImVec4(c.x, c.y, c.z, a/255.0f);
}


void duCalcBoxColors(ImVec4* colors, ImVec4 colTop, ImVec4 colSide);

void duDebugDrawCylinderWire(ImDrawList* dd, float minx, float miny, float minz,
							 float maxx, float maxy, float maxz, ImVec4 col, const float lineWidth);

void duDebugDrawBoxWire(ImDrawList* dd, float minx, float miny, float minz,
						float maxx, float maxy, float maxz, ImVec4 col, const float lineWidth);

void duDebugDrawArc(ImDrawList* dd, const float x0, const float y0, const float z0,
					const float x1, const float y1, const float z1, const float h,
					const float as0, const float as1, ImVec4 col, const float lineWidth);

void duDebugDrawArrow(ImDrawList* dd, const float x0, const float y0, const float z0,
					  const float x1, const float y1, const float z1,
					  const float as0, const float as1, ImVec4 col, const float lineWidth);

void duDebugDrawCircle(ImDrawList* dd, const float x, const float y, const float z,
					   const float r, ImVec4 col, const float lineWidth);

void duDebugDrawCross(ImDrawList* dd, const float x, const float y, const float z,
					  const float size, ImVec4 col, const float lineWidth);

void duDebugDrawBox(ImDrawList* dd, float minx, float miny, float minz,
					float maxx, float maxy, float maxz, const ImVec4* fcol);

void duDebugDrawCylinder(ImDrawList* dd, float minx, float miny, float minz,
						 float maxx, float maxy, float maxz, ImVec4 col);

void duDebugDrawGridXZ(ImDrawList* dd, const float ox, const float oy, const float oz,
					   const int w, const int h, const float size,
					   const ImVec4 col, const float lineWidth);


// Versions without begin/end, can be used to draw multiple primitives.
void duAppendCylinderWire(ImDrawList* dd, float minx, float miny, float minz,
						  float maxx, float maxy, float maxz, ImVec4 col);

void duAppendBoxWire(ImDrawList* dd, float minx, float miny, float minz,
					 float maxx, float maxy, float maxz, ImVec4 col);

void duAppendBoxPoints(ImDrawList* dd, float minx, float miny, float minz,
					   float maxx, float maxy, float maxz, ImVec4 col);

void duAppendArc(ImDrawList* dd, const float x0, const float y0, const float z0,
				 const float x1, const float y1, const float z1, const float h,
				 const float as0, const float as1, ImVec4 col);

void duAppendArrow(ImDrawList* dd, const float x0, const float y0, const float z0,
				   const float x1, const float y1, const float z1,
				   const float as0, const float as1, ImVec4 col);

void duAppendCircle(ImDrawList* dd, const float x, const float y, const float z,
					const float r, ImVec4 col);

void duAppendCross(ImDrawList* dd, const float x, const float y, const float z,
				   const float size, ImVec4 col);

void duAppendBox(ImDrawList* dd, float minx, float miny, float minz,
				 float maxx, float maxy, float maxz, const ImVec4* fcol);

void duAppendCylinder(ImDrawList* dd, float minx, float miny, float minz,
					  float maxx, float maxy, float maxz, ImVec4 col);


class duDisplayList : public duDebugDraw
{
	float* m_pos;
	ImVec4* m_color;
	int m_size;
	int m_cap;

	duDebugDrawPrimitives m_prim;
	float m_primSize;
	bool m_depthMask;
	
	void resize(int cap);
	
public:
	duDisplayList(int cap = 512);
	virtual ~duDisplayList();
	virtual void depthMask(bool state);
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float x, const float y, const float z, ImVec4 color);
	virtual void vertex(const float* pos, ImVec4 color);
	virtual void end();
	void clear();
	void draw(ImDrawList* dd);
private:
	// Explicitly disabled copy constructor and copy assignment operator.
	duDisplayList(const duDisplayList&);
	duDisplayList& operator=(const duDisplayList&);
};


}
}