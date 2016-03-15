#include "stdafx.h"
#include "GObject.h"

GObject::GObject()
{
	m_type = TYPE_NONE;
	m_name = "";
}


GObject::~GObject()
{
}

int GObject::GetType()
{
	return m_type;
}

void GObject::SetType(OBJECT_TYPE type)
{
	if (type > TYPE_NONE && type <= TYPE_CLOUD)
		m_type = type;
	else
		m_type = TYPE_NONE;
}

std::string GObject::GetName()
{
	return m_name;
}

void GObject::SetName(std::string name)
{
	m_name = name;
}
