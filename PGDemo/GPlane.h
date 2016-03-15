#pragma once
#include "GObject.h"
#include <vector>
class GPlane:GObject
{
public:
	GPlane();
	GPlane(GPoint pnt, GPoint nor);
	~GPlane();

	GPlane& operator=(GPlane p);

	GPoint GetProjectPnt(GPoint pnt);

	void UpdateVertexs(float len = 0.5);

	void Rotate(GPoint rot_axis_pnt, GPoint rot_axis_dir, double angle);

	void Translate(GPoint trans);

public:
	GPoint m_PlanePnt;
	GPoint m_PlaneNor;
	std::vector<GPoint> m_Vertexs;
};

