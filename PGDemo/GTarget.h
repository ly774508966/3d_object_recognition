#pragma once
#include <vector>
#include "GCloud.h"
enum TARGET_TYPE
{
	TARGET_NONE = 0,
	TARGET_BOTTLE = 1,
	TARGET_BOX = 2
};

class GTarget
{
public:
	GTarget();
	~GTarget();
	GTarget& operator=(GTarget c);

public:
	std::vector<int> m_indexList;
	GCloud m_cld;
	int m_sampleNo;
	int m_type;
	GPoint m_center;
	float m_minorRadius;
	float m_majorRadius;
	float m_length;
	float m_width;
	float m_height;
	std::vector<GPoint> m_vertexs;
};

