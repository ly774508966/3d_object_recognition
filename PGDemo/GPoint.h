#pragma once
#define PI 3.1415926

class GPoint
{
public:
	GPoint();

	GPoint(const int&x, const int&y, const int&z);

	GPoint(const float&x, const float&y, const float&z);

	GPoint(const double&x, const double&y, const double&z);

	GPoint(const float* pnt);

	GPoint(const double* pnt);

	GPoint(const GPoint& pnt);

	~GPoint();

	const float& operator[](const int& index) const;

	float& operator[](const int& index);

	GPoint& operator=(const GPoint& p);

	GPoint& operator+=(const GPoint& p);

	GPoint& operator-=(const GPoint& p);

	GPoint& operator*=(const float& a);

	GPoint& operator/=(const float& a);

	GPoint operator+(const GPoint& p) const;

	GPoint operator-(const GPoint& p) const;

	GPoint operator*(float a) const;

	GPoint operator/(float a) const;

	//dot product
	double operator*(const GPoint& p) const;

	//change sign
	GPoint operator-() const;

	GPoint operator^(const GPoint& p) const;

	bool operator==(const GPoint& p) const;

	void Normalize();

	void NormalizeNormal();

	double Length();

	double Length2D();

	void Rotate(GPoint pnt, GPoint dir,	double rad);

	void SetColor(GPoint p);

	//calculate rotation angle between two vectors
	//@param[in] to_vec : target vector
	//@param[in] normal : axis of rotation
	//@return : rotation angle in rad
	double Angle2Vector(GPoint to_vec, GPoint normal);

	//3D coordinate
	float m[3];

	//point normal
	float normal[3];

	//color
	float color[3];

};

