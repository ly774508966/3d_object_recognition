#include "stdafx.h"
#include "GPlane.h"

GPlane::GPlane()
{
}

GPlane::GPlane(GPoint pnt, GPoint nor)
{
	m_PlanePnt = pnt;
	m_PlaneNor = nor;
	m_PlaneNor.Normalize();
}

GPlane::~GPlane()
{
}

GPlane& GPlane::operator=(GPlane p)
{
	if (&p != this)
	{
		m_PlanePnt = p.m_PlanePnt;
		m_PlaneNor = p.m_PlaneNor;
		m_Vertexs = p.m_Vertexs;
	}
	return *this;
}

GPoint GPlane::GetProjectPnt(GPoint pnt)
{
	m_PlaneNor.Normalize();
	double a = m_PlaneNor[0];
	double b = m_PlaneNor[1];
	double c = m_PlaneNor[2];
	double d = -a*m_PlanePnt[0] - b*m_PlanePnt[1] - c*m_PlanePnt[2];
	double dist = a*pnt[0] + b*pnt[1] + c*pnt[2] + d;
	GPoint proj_pnt;
	proj_pnt[0] = pnt[0] - a*dist;
	proj_pnt[1] = pnt[1] - b*dist;
	proj_pnt[2] = pnt[2] - c*dist;

	return proj_pnt;
}

void GPlane::UpdateVertexs(float len)
{
	m_Vertexs.clear();
	GPoint pnt = GetProjectPnt(GPoint(m_PlanePnt[0] + len, m_PlanePnt[1], m_PlanePnt[2]));
	m_Vertexs.push_back(pnt);
	for (int i = 0; i < 3; i++)
	{
		pnt.Rotate(m_PlanePnt, m_PlaneNor, 0.5*PI);
		m_Vertexs.push_back(pnt);
	}
}

void GPlane::Rotate(GPoint rot_axis_pnt, GPoint rot_axis_dir, double angle)
{
	rot_axis_dir.Normalize();
	double s_theta, c_theta, vers_theta;
	if (std::fabs(angle) < 1e-6)
		return;
	s_theta = std::sin(angle);
	c_theta = std::cos(angle);
	vers_theta = 1 - c_theta;
	double trans[3][3];
	double x, y, z;
	x = rot_axis_dir[0];
	y = rot_axis_dir[1];
	z = rot_axis_dir[2];
	trans[0][0] = x*x*vers_theta + c_theta;
	trans[0][1] = y*x*vers_theta - z*s_theta;
	trans[0][2] = z*x*vers_theta + y*s_theta;
	trans[1][0] = x*y*vers_theta + z*s_theta;
	trans[1][1] = y*y*vers_theta + c_theta;
	trans[1][2] = y*z*vers_theta - x*s_theta;
	trans[2][0] = x*z*vers_theta - y*s_theta;
	trans[2][1] = y*z*vers_theta + x*s_theta;
	trans[2][2] = z*z*vers_theta + c_theta;
	int num = m_Vertexs.size();
	float pnt[3];
	for (int i = 0; i < num; i++)
	{
		pnt[0] = m_Vertexs[i][0] - rot_axis_pnt[0];
		pnt[1] = m_Vertexs[i][1] - rot_axis_pnt[1];
		pnt[2] = m_Vertexs[i][2] - rot_axis_pnt[2];
		m_Vertexs[i][0] = trans[0][0] * pnt[0] + trans[0][1] * pnt[1] + trans[0][2] * pnt[2];
		m_Vertexs[i][1] = trans[1][0] * pnt[0] + trans[1][1] * pnt[1] + trans[1][2] * pnt[2];
		m_Vertexs[i][2] = trans[2][0] * pnt[0] + trans[2][1] * pnt[1] + trans[2][2] * pnt[2];
		m_Vertexs[i][0] += rot_axis_pnt[0];
		m_Vertexs[i][1] += rot_axis_pnt[1];
		m_Vertexs[i][2] += rot_axis_pnt[2];
	}

	pnt[0] = m_PlanePnt[0] - rot_axis_pnt[0];
	pnt[1] = m_PlanePnt[1] - rot_axis_pnt[1];
	pnt[2] = m_PlanePnt[2] - rot_axis_pnt[2];
	m_PlanePnt[0] = trans[0][0] * pnt[0] + trans[0][1] * pnt[1] + trans[0][2] * pnt[2];
	m_PlanePnt[1] = trans[1][0] * pnt[0] + trans[1][1] * pnt[1] + trans[1][2] * pnt[2];
	m_PlanePnt[2] = trans[2][0] * pnt[0] + trans[2][1] * pnt[1] + trans[2][2] * pnt[2];
	m_PlanePnt[0] += rot_axis_pnt[0];
	m_PlanePnt[1] += rot_axis_pnt[1];
	m_PlanePnt[2] += rot_axis_pnt[2];

	pnt[0] = m_PlaneNor[0];
	pnt[1] = m_PlaneNor[1];
	pnt[2] = m_PlaneNor[2];
	m_PlaneNor[0] = trans[0][0] * pnt[0] + trans[0][1] * pnt[1] + trans[0][2] * pnt[2];
	m_PlaneNor[1] = trans[1][0] * pnt[0] + trans[1][1] * pnt[1] + trans[1][2] * pnt[2];
	m_PlaneNor[2] = trans[2][0] * pnt[0] + trans[2][1] * pnt[1] + trans[2][2] * pnt[2];
}

void GPlane::Translate(GPoint trans)
{
	trans.normal[0] = 0;
	trans.normal[1] = 0;
	trans.normal[2] = 0;
	for (int i = 0; i < m_Vertexs.size(); i++)
		m_Vertexs[i] += trans;

	m_PlanePnt += trans;
}