
// PGDemoDlg.h : header file
//

#pragma once
#include <Kinect.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glut.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "GCloud.h"
#include "GPlane.h"
#include "ConfigIO.h"
#include "GTarget.h"
enum FLAG_TYPE
{
	FLAG_INVALID_BACKGROUND = 0,
	FLAG_VALID_BACKGROUND = 1,
	FLAG_VALID_DATA = 2
};
// CPGDemoDlg dialog
class CPGDemoDlg : public CDialogEx
{
// Construction
public:
	CPGDemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PGDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	//opengl
	bool SetWindowPixelFormat(HDC hdc);
	bool InitializeOpenGL(HDC hdc);
	void RenderScene();

	//kinect
	bool CreateFirstConnected();

	//thread to receive kinect data
	static DWORD WINAPI ProcessThread(LPVOID lpParam);
	DWORD WINAPI ProcessThread();

	// convert kinect color frame to opencv
	// @param[in] color : kinect color frame
	// @param[out] frame : opencv color frame
	// return value : true if success
	bool GetColorFrame(IColorFrame* color, cv::Mat* frame);

	// convert kinect depth frame to opencv
	// @param[in] depth : kinect depth frame
	// @param[out] frame : opencv depth frame
	// return value : true if success
	bool GetDepthFrame(IDepthFrame* depth, cv::Mat* frame);

	// convert kinect depth frame to 3D point cloud
	// @param[in] depth : kinect depth frame
	// @param[out] cld : reconstructed 3D point cloud
	void GetSceneCld(IDepthFrame* depth, GCloud* cld);

	//save depth/color frames and point cloud with default names
	void SaveScene();

	//load depth frame, color frame and point cloud
	void LoadScene(CString fn);

	// save valid background pixels as image
	// @param[in] fn : file path
	// @param[in] ind_list : index of valid backgournd pixels
	void SaveSegmentImg(CString fn, std::vector<int> ind_list);

	// load valid background pixels from image
	// @param[in] fn : file path
	// @param[out] ind_list : index of valid background pixels
	bool LoadSegmentImg(CString fn, std::vector<int>& ind_list);

	void ShowScaleImage(CString wnd_name, cv::Mat* img, double scale = 0.5);

	// update m_validFlags
	// @param[in] ind_list : pixels to be updated
	// @param[in] flag : pixel value to be updated
	void UpdateValidFlags(std::vector<int> ind_list, int flag = 1);

	// reset m_validFlags, 0 as invalid pixel
	// @param[in] ind_list : pixels to be reset, others will be set as 0
	// @param[in] flag : pixel value to be reset
	void ResetValidFlags(std::vector<int> ind_list, int flag = 1);

	//process point cloud and do target detection
	void ProcessScene();

	void InitializeScene();

	//load reference target and corresponding target images
	bool LoadRefTargetList();

	// paint target extraction and recognition results in color frame
	// @param[in/out] tar : detected target
	bool PaintTargetInColor(GTarget& tar);

	// recognize extracted target according to reference target
	// @param[in] tar : detected target
	int RecognizeTarget(GTarget tar);

	// find corresponding point coordinates in color frame for a point in depth frame, when kinect is not connected
	// @param[in] pt : point cooridnates in depth frame
	// return value : point coordinates in color frame
	GPoint ConvertPntDepth2ColorStatic(GPoint pt);

	// paint a sub image in the color frame
	// @param[in] img_name : image name
	// @param[in] px : x coordinate of sub image's top left corner
	// @param[in] py : y coordinate of sub image's top left corner
	void PaintSubImg(CString img_name, int px, int py);

	// paint a sub image in the color frame
	// @param[in] ref_no : reference target index
	// @param[in] px : x coordinate of sub image's top left corner
	// @param[in] py : y coordinate of sub image's top left corner
	void PaintSubImg(int ref_no, int px, int py);

	//save coordinate mapping between depth frame and color frame
	void SaveDepth2ColorMapping();

	//preprocess coordinate mapping between depth frame and color frame
	//use interpolation for pixels of invalid depth value 
	void PreprocessDepth2ColorMapping(CString fpath);

public:
	//kinect
	IKinectSensor* m_Kinect;
	bool m_bKinectConnected;
	bool m_bKinectStreaming;
	IColorFrameReader* m_colorReader;
	IDepthFrameReader* m_depthReader;
	RGBQUAD* m_pColorRGBX;
	RGBQUAD* m_pDepthRGBX;
	ICoordinateMapper* m_mapper;
	USHORT m_minDepth;
	USHORT m_maxDepth;

	//handler
	HANDLE m_hProcessThread;
	HANDLE m_hPaintMutex;

	//kinect color frame
	cv::Mat m_colorMat;

	//kinect depth frame
	cv::Mat m_depthMat;
	cv::Mat m_depthMat16;

	//kinect 3D point cloud
	GCloud m_SceneCld;
	float m_minZLimit;
	float m_maxZLimit;

	//opengl
	bool m_bMouseLeftDown;
	bool m_bMouseRightDown;
	float m_MouseX;
	float m_MouseY;
	float m_RotX, m_RotY;
	float m_TransX, m_TransY;
	float m_scale;

	//size of color/depth frame
	int m_colorWidth, m_colorHeight, m_depthWidth, m_depthHeight;

	//opengl plan render list
	std::vector<GPlane> m_planeList;

	//configuration I/O
	ConfigIO m_config;

	//background calibration
	//assume samples are placed on a flat desk, after coordinate transform, normal of desk plane is along +Z
	GPoint m_bkTrans, m_bkRotAxis;
	double m_bkRotRad;

	//index list of valid points
	std::vector<int> m_inlierList;

	//indicate whehter each pixel is valid
	int* m_validFlags;

	//targets extracted from kinect data
	std::vector<GTarget> m_targets;//extraction results

	//opengl, bounding box render list
	std::vector<GPoint> m_targetBoxPnts;

	//reference target list
	std::vector<GTarget> m_refTargets;

	//flag to save kinect frame
	bool m_saveFrame;

	bool m_saveResult;

	//default path for reference target
	CString m_refDirectory;
	CString m_path;

	//image list for reference targets
	std::vector<cv::Mat> m_refImgList;

	//color list for opengl display
	std::vector<GPoint> m_colorList;

	//mapping betwen depth pixel to color pixel
	//this result is used for mapping between 3D data and color frame when kinect is not connected
	std::vector<GPoint> m_depth2ColorMapper;

// Implementation
protected:
	HICON m_hIcon;

	//opengl
	HDC m_dc;
	HGLRC m_rc;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonCalibrate();
	afx_msg void OnBnClickedButtonTest();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnBnClickedButtonProcess();
	afx_msg void OnBnClickedButtonRestoreview();
	afx_msg void OnBnClickedButtonRecognitionresult();
};
