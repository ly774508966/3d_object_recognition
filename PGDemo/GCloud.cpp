#include "stdafx.h"
#include "GCloud.h"
#include <Eigen\dense>
#include <fstream>
#include "DebugLog.h"
GCloud::GCloud()
{
}


GCloud::~GCloud()
{
}

GCloud& GCloud::operator=(GCloud c)
{
	if (&c != this)
	{
		this->Initialize();
		if (c.GetSize() > 0)
		{
			m_pnts = c.m_pnts;
			m_validList = c.m_validList;
		}
		m_boxMaxPnt = c.m_boxMaxPnt;
		m_boxMinPnt = c.m_boxMinPnt;
	}
return *this;
}

GCloud& GCloud::operator+(GCloud c)
{
	if (c.GetSize() > 0)
		m_pnts.insert(m_pnts.end(), c.m_pnts.begin(), c.m_pnts.end());
	return *this;
}

GPoint& GCloud::operator[](int index)
{
	if (index > 0 && index < GetSize())
		return m_pnts[index];
	return GPoint(0, 0, 0);
}

void GCloud::AddPoint(GPoint p)
{
	m_pnts.push_back(p);
}

int GCloud::GetSize()
{
	return m_pnts.size();
}

void GCloud::Resize(int num)
{
	m_pnts.clear();
	m_pnts.resize(num);
}

void GCloud::Initialize()
{
	m_pnts.clear();
	m_validList.clear();
}

void GCloud::Rotate(GPoint rot_axis_pnt, GPoint rot_axis_dir, double angle)
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
	int num = GetSize();
	float pnt[3];
	for (int i = 0; i < num; i++)
	{
		pnt[0] = m_pnts[i][0] - rot_axis_pnt[0];
		pnt[1] = m_pnts[i][1] - rot_axis_pnt[1];
		pnt[2] = m_pnts[i][2] - rot_axis_pnt[2];
		m_pnts[i][0] = trans[0][0] * pnt[0] + trans[0][1] * pnt[1] + trans[0][2] * pnt[2];
		m_pnts[i][1] = trans[1][0] * pnt[0] + trans[1][1] * pnt[1] + trans[1][2] * pnt[2];
		m_pnts[i][2] = trans[2][0] * pnt[0] + trans[2][1] * pnt[1] + trans[2][2] * pnt[2];
		m_pnts[i][0] += rot_axis_pnt[0];
		m_pnts[i][1] += rot_axis_pnt[1];
		m_pnts[i][2] += rot_axis_pnt[2];

		pnt[0] = m_pnts[i].normal[0];
		pnt[1] = m_pnts[i].normal[1];
		pnt[2] = m_pnts[i].normal[2];
		m_pnts[i].normal[0] = trans[0][0] * pnt[0] + trans[0][1] * pnt[1] + trans[0][2] * pnt[2];
		m_pnts[i].normal[1] = trans[1][0] * pnt[0] + trans[1][1] * pnt[1] + trans[1][2] * pnt[2];
		m_pnts[i].normal[2] = trans[2][0] * pnt[0] + trans[2][1] * pnt[1] + trans[2][2] * pnt[2];
	}
}

void GCloud::Translate(GPoint trans)
{
	trans.normal[0] = 0;
	trans.normal[1] = 0;
	trans.normal[2] = 0;
	for (int i = 0; i < GetSize(); i++)
		m_pnts[i] += trans;
}

void GCloud::Save(std::string fn)
{
	CStdioFile fp;
	if (fp.Open(fn.c_str(), CStdioFile::modeCreate | CStdioFile::modeWrite))
	{
		CString txt;
		for (int i = 0; i < GetSize(); i++)
		{
			txt.Format("%.6f %.6f %.6f %.6f %.6f %.6f\n", m_pnts[i][0], m_pnts[i][1], m_pnts[i][2], m_pnts[i].normal[0], m_pnts[i].normal[1], m_pnts[i].normal[2]);
			fp.WriteString(txt);
		}
		fp.Close();
	}
}

void GCloud::Load(std::string fn)
{
	FILE* fp = fopen(fn.c_str(), "r");
	if (fp == nullptr)
		return;
	Initialize();
	char line[256];
	float px, py, pz;
	float nx, ny, nz;
	while (fgets(line, 256, fp))
	{
		if (sscanf(line, "%f %f %f %f %f %f\n", &px, &py, &pz, &nx, &ny, &nz) == 6)
		{
			GPoint pnt(px, py, pz);
			pnt.normal[0] = nx;
			pnt.normal[1] = ny;
			pnt.normal[2] = nz;
			AddPoint(pnt);
		}
	}
	fclose(fp);
}

GPoint GCloud::GetMassCenter()
{
	if (m_validList.size() == 0)
		FilterPnts();
	
	GPoint pnt;

	int num = m_validList.size();
	for (int i = 0; i < num; i++)
	{
		pnt += m_pnts[m_validList[i]];
	}
	if (num > 0)
		pnt /= num;
	return pnt;
}

void GCloud::UpdateBndBox()
{
	if (m_validList.size() == 0)
		FilterPnts();
	if (m_validList.size() == 0)
		return;

	m_boxMinPnt = m_pnts[m_validList[0]];
	m_boxMaxPnt = m_pnts[m_validList[0]];
	for (int i = 0; i < m_validList.size(); i++)
	{
		GPoint pnt = m_pnts[m_validList[i]];
		for (int j = 0; j < 3; j++)
		{
			m_boxMinPnt[j] = pnt[j] < m_boxMinPnt[j] ? pnt[j] : m_boxMinPnt[j];
			m_boxMaxPnt[j] = pnt[j] > m_boxMaxPnt[j] ? pnt[j] : m_boxMaxPnt[j];
		}
	}
}

void GCloud::FilterPnts()
{
	m_validList.clear();
	for (int i = 0; i < GetSize(); i++)
	{
		GPoint pnt = m_pnts[i];
		if (pnt[0] == 0 && pnt[1] == 0 && pnt[2] == 0)
			continue;
		else
			m_validList.push_back(i);
	}
}

void GCloud::Fit2Plane(GPoint& pnt, GPoint& dir)
{
	if (m_validList.size() == 0)
		FilterPnts();

	int num = m_validList.size();
	pnt = GetMassCenter();
	if (num < 3)
	{		
		dir = GPoint(0, 0, 1);
		return;
	}
	GCloud cld;
	for (int i = 0; i < m_validList.size(); i++)
		cld.AddPoint(m_pnts[m_validList[i]]);
	cld.Translate(-pnt);
	Eigen::MatrixXd m(3, 3);
	m = Eigen::MatrixXd::Zero(3, 3);
	for (int i = 0; i < num; i++)
	{
		m(0, 0) += cld[i][0] * cld[i][0];
		m(0, 1) += cld[i][0] * cld[i][1];
		m(0, 2) += cld[i][0] * cld[i][2];
		m(1, 0) += cld[i][1] * cld[i][0];
		m(1, 1) += cld[i][1] * cld[i][1];
		m(1, 2) += cld[i][1] * cld[i][2];
		m(2, 0) += cld[i][2] * cld[i][0];
		m(2, 1) += cld[i][2] * cld[i][1];
		m(2, 2) += cld[i][2] * cld[i][2];
	}
	Eigen::EigenSolver<Eigen::MatrixXd> es(m);
	double min_eigen = es.eigenvalues()[0].real();
	int min_eigen_index = 0;
	for (int i = 1; i < 3; i++)
	{
		double new_eigen = es.eigenvalues()[i].real();
		if (new_eigen < min_eigen)
		{
			min_eigen = new_eigen;
			min_eigen_index = i;
		}
	}
	Eigen::VectorXcd v = es.eigenvectors().col(min_eigen_index);
	dir[0] = v(0).real();
	dir[1] = v(1).real();
	dir[2] = v(2).real();
	dir.Normalize();
	if (dir[2]>0)
		dir = -dir;

	double d = -dir[0]*pnt[0] - dir[1]*pnt[1] - dir[2]*pnt[2];
	GPoint new_plane_pnt(0, 0, 0);
	new_plane_pnt[2] = -d / dir[2];
	pnt = new_plane_pnt;
}

std::vector<int> GCloud::Fit2PlaneRejection(GPoint& pnt, GPoint& dir, double span)
{
	Fit2Plane(pnt, dir);
	std::vector<int> inlier_list;
	if (m_validList.size() < 4)
		return inlier_list;
	int counter = 0;
	GPoint new_pnt, new_dir;
	int prev_cld_num = m_validList.size();
	
	while (counter<10)
	{
		GCloud new_cld;
		inlier_list.clear();
		for (int i = 0; i < m_validList.size(); i++)
		{
			double dist = std::fabs((m_pnts[m_validList[i]]-pnt)*dir);
			if (dist < span)
			{
				new_cld.AddPoint(m_pnts[m_validList[i]]);
				inlier_list.push_back(m_validList[i]);
			}
		}
		if (new_cld.GetSize() == 0)
			break;
		if (new_cld.GetSize() == prev_cld_num)
			break;
		prev_cld_num = new_cld.GetSize();
		new_cld.Fit2Plane(new_pnt, new_dir);
		pnt = new_pnt;
		dir = new_dir;
		counter++;
	}

	return inlier_list;
}
