#pragma once
#include "GPoint.h"
class ConfigIO
{
public:
	ConfigIO();
	~ConfigIO();

	//save calibration results for tabletop
	void SaveBkCalibration(GPoint trans, GPoint rot_dir, double rot_rad);

	//load calibraiton results for tabletop
	void LoadBkCalibration(GPoint& trans, GPoint& rot_dir, double& rot_rad);

	void UpdateFilePath(CString fpath);

	//load nearest and farest limit for z value
	void LoadKinectZLimit(float& minLimit, float& maxLimit);

private:
	CString m_iniFile;
};

