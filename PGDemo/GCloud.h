#pragma once
#include "GObject.h"
#include <vector>

class GCloud:public GObject
{
public:
	GCloud();
	~GCloud();

	GCloud& operator=(GCloud c);

	GCloud& operator+(GCloud c);

	GPoint& operator[](int index);

	//add a point at the end of the cloud
	void AddPoint(GPoint p);

	//get number of points
	int GetSize();

	void Resize(int num);

	//cloud initialization
	void Initialize();

	//rotate a poitn cloud along specified rotary axis
	//@param[in] rot_axis_pnt : a point on the rotary axis
	//@param[in] rot_axis_dir : direction of rotary axis
	//@param[in] angle : rotary angle in rad
	void Rotate(GPoint rot_axis_pnt, GPoint rot_axis_dir, double angle);

	void Translate(GPoint trans);

	void Save(std::string fn);

	void Load(std::string fn);

	void Fit2Plane(GPoint& pnt, GPoint& dir);

	std::vector<int> Fit2PlaneRejection(GPoint& pnt, GPoint& dir, double span = 0.01);

	GPoint GetMassCenter();

	void UpdateBndBox();

	void FilterPnts();

public:
	std::vector<GPoint> m_pnts;
	GPoint m_boxMinPnt, m_boxMaxPnt;
	std::vector<int> m_validList;
};

