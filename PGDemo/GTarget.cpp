#include "stdafx.h"
#include "GTarget.h"


GTarget::GTarget()
{
}


GTarget::~GTarget()
{
}

GTarget& GTarget::operator=(GTarget c)
{
	if (&c != this)
	{
		m_indexList = c.m_indexList;
		m_sampleNo = c.m_sampleNo;
		m_cld = c.m_cld;
		m_type = c.m_type;
		m_center = c.m_center;
		m_minorRadius = c.m_minorRadius;
		m_majorRadius = c.m_majorRadius;
		m_length = c.m_length;
		m_width = c.m_width;
		m_height = c.m_height;
		m_vertexs = c.m_vertexs;
	}
	return *this;
}