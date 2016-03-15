#include "stdafx.h"
#include "ConfigIO.h"


ConfigIO::ConfigIO()
{
	m_iniFile = "c:\\tmp\\kinect\\kinectV2_config.ini";
}


ConfigIO::~ConfigIO()
{
}

void ConfigIO::UpdateFilePath(CString fpath)
{
	m_iniFile = fpath + "\\kinectV2_config.ini";
}

void ConfigIO::SaveBkCalibration(GPoint trans, GPoint rot_dir, double rot_rad)
{
	TCHAR Value[MAX_PATH];
	sprintf(Value, "%f", trans[0]);
	WritePrivateProfileString("BkCloud", "TRANS_X", Value, m_iniFile);
	sprintf(Value, "%f", trans[1]);
	WritePrivateProfileString("BkCloud", "TRANS_Y", Value, m_iniFile);
	sprintf(Value, "%f", trans[2]);
	WritePrivateProfileString("BkCloud", "TRANS_Z", Value, m_iniFile);
	sprintf(Value, "%f", rot_dir[0]);
	WritePrivateProfileString("BkCloud", "ROT_X", Value, m_iniFile);
	sprintf(Value, "%f", rot_dir[1]);
	WritePrivateProfileString("BkCloud", "ROT_Y", Value, m_iniFile);
	sprintf(Value, "%f", rot_dir[2]);
	WritePrivateProfileString("BkCloud", "ROT_Z", Value, m_iniFile);
	sprintf(Value, "%f", rot_rad);
	WritePrivateProfileString("BkCloud", "ROT_RAD", Value, m_iniFile);
}

void ConfigIO::LoadBkCalibration(GPoint& trans, GPoint& rot_dir, double& rot_rad)
{
	TCHAR str[MAX_PATH];
	GetPrivateProfileString("BkCloud", "TRANS_X", "0", str, MAX_PATH, m_iniFile);
	trans[0] = atof(str);
	GetPrivateProfileString("BkCloud", "TRANS_Y", "0", str, MAX_PATH, m_iniFile);
	trans[1] = atof(str);
	GetPrivateProfileString("BkCloud", "TRANS_Z", "0", str, MAX_PATH, m_iniFile);
	trans[2] = atof(str);
	GetPrivateProfileString("BkCloud", "ROT_X", "0", str, MAX_PATH, m_iniFile);
	rot_dir[0] = atof(str);
	GetPrivateProfileString("BkCloud", "ROT_Y", "0", str, MAX_PATH, m_iniFile);
	rot_dir[1] = atof(str);
	GetPrivateProfileString("BkCloud", "ROT_Z", "0", str, MAX_PATH, m_iniFile);
	rot_dir[2] = atof(str);
	rot_dir.Normalize();
	GetPrivateProfileString("BkCloud", "ROT_RAD", "0", str, MAX_PATH, m_iniFile);
	rot_rad = atof(str);
}

void ConfigIO::LoadKinectZLimit(float& minLimit, float& maxLimit)
{
	TCHAR str[MAX_PATH];
	GetPrivateProfileString("Kinect", "NEAR_LIMIT", "0", str, MAX_PATH, m_iniFile);
	minLimit = atof(str);
	GetPrivateProfileString("Kinect", "FAR_LIMIT", "0", str, MAX_PATH, m_iniFile);
	maxLimit = atof(str);	
}

