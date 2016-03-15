#include "stdafx.h"
#include "GPoint.h"
#include <cmath>

GPoint::GPoint()
{
	m[0] = 0;
	m[1] = 0;
	m[2] = 0;
	normal[0] = 0;
	normal[1] = 0;
	normal[2] = 0;
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
}

GPoint::GPoint(const int&x, const int&y, const int&z)
{
	m[0] = (float)x;
	m[1] = (float)y;
	m[2] = (float)z;
	normal[0] = 0;
	normal[1] = 0;
	normal[2] = 0;
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
}

GPoint::GPoint(const float&x, const float&y, const float&z)
{
	m[0] = x;
	m[1] = y;
	m[2] = z;
	normal[0] = 0;
	normal[1] = 0;
	normal[2] = 0;
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
}

GPoint::GPoint(const double&x, const double&y, const double&z)
{
	m[0] = (float)x;
	m[1] = (float)y;
	m[2] = (float)z;
	normal[0] = 0;
	normal[1] = 0;
	normal[2] = 0;
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
}

GPoint::GPoint(const float* pnt)
{
	for (int i = 0; i < 3; i++)
	{
		m[i] = pnt[i];
	}
	normal[0] = 0;
	normal[1] = 0;
	normal[2] = 0;
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
}

GPoint::GPoint(const double* pnt)
{
	for (int i = 0; i < 3; i++)
	{
		m[i] = (float)pnt[i];
	}
	normal[0] = 0;
	normal[1] = 0;
	normal[2] = 0;
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
}

GPoint::GPoint(const GPoint& pnt)
{
	for (int i = 0; i < 3; i++)
	{
		m[i] = pnt[i];
		normal[i] = pnt.normal[i];
	}
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;
}

GPoint::~GPoint()
{
}

const float& GPoint::operator[](const int& index) const
{
	if (index < 3)
		return m[index];
	else
		return m[0];
}

float& GPoint::operator[](const int& index)
{
	if (index < 3)
		return m[index];
	else
		return m[0];
}

GPoint& GPoint::operator=(const GPoint& p)
{
	if (&p == this)
		return *this;

	for (int i = 0; i < 3; i++)
	{
		m[i] = p[i];
		normal[i] = p.normal[i];
		color[i] = p.color[i];
	}
	return *this;
}

GPoint& GPoint::operator+=(const GPoint& p)
{
	for (int i = 0; i < 3; i++)
	{
		m[i] += p[i];
		normal[i] += p.normal[i];
	}

	NormalizeNormal();

	return *this;
}

GPoint& GPoint::operator-=(const GPoint& p)
{
	for (int i = 0; i < 3; i++)
	{
		m[i] -= p[i];
		normal[i] -= p.normal[i];
	}

	NormalizeNormal();

	return *this;
}

GPoint& GPoint::operator*=(const float& a)
{
	m[0] *= a;
	m[1] *= a;
	m[2] *= a;
	return *this;
}

GPoint& GPoint::operator/=(const float& a)
{
	if (fabs(a) < 1e-6)
		return *this;

	m[0] /= a;
	m[1] /= a;
	m[2] /= a;
	return *this;
}

GPoint GPoint::operator+(const GPoint& p) const
{
	GPoint pnt(*this);
	pnt += p;
	return pnt;
}

GPoint GPoint::operator-(const GPoint& p) const
{
	GPoint pnt(*this);
	pnt -= p;
	return pnt;
}

GPoint GPoint::operator*(float a) const
{
	GPoint pnt(*this);
	pnt *= a;
	return pnt;
}

GPoint GPoint::operator/(float a) const
{
	GPoint pnt(*this);
	pnt /= a;
	return pnt;
}

double GPoint::operator*(const GPoint& p) const
{
	return (double)(m[0] * p[0] + m[1] * p[1] + m[2] * p[2]);
}

GPoint GPoint::operator-() const
{
	GPoint pnt;
	pnt -= *this;
	return pnt;
}

GPoint GPoint::operator^(const GPoint& p) const
{
	return GPoint(m[1] * p[2] - m[2] * p[1],
				  m[2] * p[0] - m[0] * p[2],
				  m[0] * p[1] - m[1] * p[0]);
}

bool GPoint::operator==(const GPoint& p) const
{
	GPoint v = *this - p;
	if (v.Length() < 1e-6)
		return true;
	else
		return false;
}

void GPoint::Normalize()
{
	double mod = std::sqrt(m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
	if (mod > 0)
	{
		m[0] /= (float)mod;
		m[1] /= (float)mod;
		m[2] /= (float)mod;
	}
}

void GPoint::NormalizeNormal()
{
	double mod = std::sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
	if (mod > 0)
	{
		normal[0] /= (float)mod;
		normal[1] /= (float)mod;
		normal[2] /= (float)mod;
	}
}

double GPoint::Length()
{
	return std::sqrt((*this)*(*this));
}

double GPoint::Length2D()
{
	return std::sqrt(m[0] * m[0] + m[1] * m[1]);
}

double GPoint::Angle2Vector(GPoint to_vec, GPoint normal)
{
	GPoint tmp;
	GPoint up = (*this) ^ normal;
	GPoint vp = to_vec^normal;
	up.Normalize();
	vp.Normalize();
	tmp = up^vp;
	if (tmp.Length() < 1e-6)
	{
		if (up*vp > 0)
			return 0;
		else
			return PI;
	}
	double mag = tmp*normal;
	int sign = mag > 0 ? 1 : -1;
	double t = up*vp;
	if (t > 1.0)
		t = 1.0;
	if (t < -1.0)
		t = -1.0;
	return (double)(sign*std::acos(t));
}

void GPoint::Rotate(GPoint pnt, GPoint dir, double rad)
{
	dir.Normalize();
	double s_theta, c_theta, vers_theta;
	if (std::fabs(rad) < 1e-6)
		return;
	s_theta = std::sin(rad);
	c_theta = std::cos(rad);
	vers_theta = 1 - c_theta;
	double trans[3][3];
	double x, y, z;
	x = dir[0];
	y = dir[1];
	z = dir[2];
	trans[0][0] = x*x*vers_theta + c_theta;
	trans[0][1] = y*x*vers_theta - z*s_theta;
	trans[0][2] = z*x*vers_theta + y*s_theta;
	trans[1][0] = x*y*vers_theta + z*s_theta;
	trans[1][1] = y*y*vers_theta + c_theta;
	trans[1][2] = y*z*vers_theta - x*s_theta;
	trans[2][0] = x*z*vers_theta - y*s_theta;
	trans[2][1] = y*z*vers_theta + x*s_theta;
	trans[2][2] = z*z*vers_theta + c_theta;
	float tmp_pnt[3];
	tmp_pnt[0] = m[0] - pnt[0];
	tmp_pnt[1] = m[1] - pnt[1];
	tmp_pnt[2] = m[2] - pnt[2];
	m[0] = trans[0][0] * tmp_pnt[0] + trans[0][1] * tmp_pnt[1] + trans[0][2] * tmp_pnt[2];
	m[1] = trans[1][0] * tmp_pnt[0] + trans[1][1] * tmp_pnt[1] + trans[1][2] * tmp_pnt[2];
	m[2] = trans[2][0] * tmp_pnt[0] + trans[2][1] * tmp_pnt[1] + trans[2][2] * tmp_pnt[2];
	m[0] += pnt[0];
	m[1] += pnt[1];
	m[2] += pnt[2];

	pnt[0] = normal[0];
	pnt[1] = normal[1];
	pnt[2] = normal[2];
	normal[0] = trans[0][0] * tmp_pnt[0] + trans[0][1] * tmp_pnt[1] + trans[0][2] * tmp_pnt[2];
	normal[1] = trans[1][0] * tmp_pnt[0] + trans[1][1] * tmp_pnt[1] + trans[1][2] * tmp_pnt[2];
	normal[2] = trans[2][0] * tmp_pnt[0] + trans[2][1] * tmp_pnt[1] + trans[2][2] * tmp_pnt[2];
}

void GPoint::SetColor(GPoint p)
{
	color[0] = p[0];
	color[1] = p[1];
	color[2] = p[2];
}