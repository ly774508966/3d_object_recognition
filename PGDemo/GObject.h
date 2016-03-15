#pragma once
#include <string>
#include "GPoint.h"
enum OBJECT_TYPE
{
	TYPE_NONE = 0,
	TYPE_LINE = 1,
	TYPE_CIRCLE = 2,
	TYPE_CYLINDER = 3,
	TYPE_CLOUD = 4
};

class GObject
{
public:
	GObject();
	~GObject();

	//get object type
	int GetType();

	//set object type
	void SetType(OBJECT_TYPE type);

	std::string GetName();

	void SetName(std::string name);

private:
	//object type
	OBJECT_TYPE m_type;

	std::string m_name;
};

