
// PGDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include <ctime>
#include "PGDemo.h"
#include "PGDemoDlg.h"
#include "afxdialogex.h"
#include "debuglog.h"
#include <direct.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPGDemoDlg dialog



CPGDemoDlg::CPGDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPGDemoDlg::IDD, pParent),
	m_Kinect(NULL),
	m_hProcessThread(NULL),
	m_bKinectConnected(false),
	m_bKinectStreaming(false),
	m_RotX(0),
	m_RotY(0),
	m_TransX(0),
	m_TransY(0),
	m_bMouseLeftDown(false),
	m_bMouseRightDown(false),
	m_validFlags(NULL),
	m_colorWidth(1920),
	m_colorHeight(1080),
	m_depthWidth(512),
	m_depthHeight(424),
	m_pColorRGBX(NULL),
	m_pDepthRGBX(NULL),
	m_mapper(NULL),
	m_scale(1.0),
	m_saveFrame(false),
	m_saveResult(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPGDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPGDemoDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CPGDemoDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CPGDemoDlg::OnBnClickedButtonOpen)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CPGDemoDlg::OnBnClickedButtonSave)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON_CALIBRATE, &CPGDemoDlg::OnBnClickedButtonCalibrate)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CPGDemoDlg::OnBnClickedButtonTest)
	ON_WM_MOUSEHWHEEL()
	ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(IDC_BUTTON_PROCESS, &CPGDemoDlg::OnBnClickedButtonProcess)
	ON_BN_CLICKED(IDC_BUTTON_RESTOREVIEW, &CPGDemoDlg::OnBnClickedButtonRestoreview)
	ON_BN_CLICKED(IDC_BUTTON_RECOGNITIONRESULT, &CPGDemoDlg::OnBnClickedButtonRecognitionresult)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CPGDemoDlg::OnBnClickedButtonDisconnect)
	ON_BN_CLICKED(IDC_CHECK_PROCESS, &CPGDemoDlg::OnBnClickedCheckProcess)
	ON_BN_CLICKED(IDCANCEL, &CPGDemoDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CPGDemoDlg message handlers

BOOL CPGDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_path = getenv("PGDEMO_PATH");
	if (m_path == "")
	{
		AfxMessageBox("Please set default directory 'PEDEMO_PATH' in enviromental variable");
		exit(0);
	}
	if (!PathFileExists(m_path))
	{
		AfxMessageBox("'PEDEMO_PATH' does not exist");
		exit(0);
	}
	_mkdir(m_path + "\\data");
	_mkdir(m_path + "\\result");
	_CreateLog(m_path.GetBuffer());
	m_config.UpdateFilePath(m_path);
	m_config.LoadKinectZLimit(m_minZLimit, m_maxZLimit);
	//_Write2LogS(m_path.GetBuffer());
	CWnd* wnd = GetDlgItem(IDC_OPENGL);
	m_dc = ::GetDC(wnd->m_hWnd);
	InitializeOpenGL(m_dc);
	cv::namedWindow("Color Frame");
	//cv::namedWindow("Depth Frame");
	m_colorMat.create(m_colorHeight, m_colorWidth, CV_8UC4);
	m_depthMat.create(m_depthHeight, m_depthWidth, CV_8U);
	m_depthMat16.create(m_depthHeight, m_depthWidth, CV_16U);
	m_SceneCld.Resize(m_depthWidth*m_depthHeight);
	m_validFlags = new int[m_depthWidth*m_depthHeight];
	m_pColorRGBX = new RGBQUAD[m_colorWidth * m_colorHeight];
	m_pDepthRGBX = new RGBQUAD[m_depthWidth * m_depthHeight];
	if (!LoadSegmentImg(m_path+"\\bk.jpg", m_inlierList))
	{
		_Write2LogS("CPGDemoDlg::OnInitDialog, Failed to load inlier data index list");
	}
	ResetValidFlags(m_inlierList);
	m_refDirectory = m_path+"\\sample";
	LoadRefTargetList();
	_Write2LogSI("loaded reference target number:", m_refTargets.size());
	m_colorList.push_back(GPoint(1.0, 0.0, 0.0));//0-BLUE
	m_colorList.push_back(GPoint(0.0, 0.0, 1.0));//1-RED
	m_colorList.push_back(GPoint(0.6, 0.1, 0.9));//2-PURPLE
	m_colorList.push_back(GPoint(1.0, 0.6, 0.1));//3-ORANGE
	m_colorList.push_back(GPoint(0.0, 1.0, 1.0));//4-CYAN
	m_colorList.push_back(GPoint(1.0, 0.2, 0.2));//5-BROWN
	m_colorList.push_back(GPoint(1.0, 1.0, 0.0));//6-YELLOW
	m_colorList.push_back(GPoint(1.0, 0.0, 1.0));//7-MAGENTA

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPGDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		m_bKinectStreaming = false;
		cv::imshow("Color Frame", m_colorMat);
		//ShowScaleImage("Color Frame", &m_colorMat);
		//cv::imshow("Depth Frame", m_depthMat);		
		RenderScene();
		m_bKinectStreaming = true;
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPGDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CPGDemoDlg::SetWindowPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pixelDesc = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SUPPORT_GDI,
		LPD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int pixel_format = ChoosePixelFormat(hdc, &pixelDesc);
	if (pixel_format == 0)
	{
		_Write2LogS("CPGDemoDlg::SetWindowPixelFormat, Opengl initialize failed...1");
		return false;
	}

	if (SetPixelFormat(hdc, pixel_format, &pixelDesc) == false)
	{
		_Write2LogS("CPGDemoDlg::SetWindowPixelFormat, opengl initialize failed...2");
		return false;
	}
	return true;
}

bool CPGDemoDlg::InitializeOpenGL(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	int n;
	if (!SetWindowPixelFormat(hdc))
	{
		return false;
	}
	n = GetPixelFormat(hdc);
	DescribePixelFormat(hdc, n, sizeof(pfd), &pfd);
	m_rc = wglCreateContext(hdc);
	if (m_rc == NULL)
	{
		_Write2LogS("CPGDemoDlg::InitializeOpenGL, null opengl rc");
		return false;
	}
	if (wglMakeCurrent(hdc, m_rc) == false)
	{
		_Write2LogS("CPGDemoDlg::InitializeOpenGL, opengl make current fail");
		return false;
	}
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	return true;
}

void CPGDemoDlg::RenderScene()
{
	//_Write2LogS("openge render scene");
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1.0, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	//gluLookAt(10, 10, 10, 0, 0, 0, 0, 1, 0);

	glPushMatrix();
	gluLookAt(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(m_TransX, -m_TransY, 0);
	glRotatef(m_RotX, 1.0f, 0.0f, 0.0f);
	glRotatef(m_RotY, 0.0f, 1.0f, 0.0f);
	glScalef(m_scale, m_scale, m_scale);
	glBegin(GL_LINES);
	glColor3d(1.0, 0.0, 0.0);   // X red  
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(1.0, 0.0, 0.0);
	glColor3d(0.0, 1.0, 0.0);   // Y green  
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 1.0, 0.0);
	glColor3d(0.0, 0.0, 1.0);   // Z blue  
	glVertex3d(0.0, 0.0, 0.0);
	glVertex3d(0.0, 0.0, 1.0);
	glEnd();
	//_Write2LogSI("cloud size", m_SceneCld.GetSize());
	if (m_SceneCld.GetSize() > 0)
	{
		glBegin(GL_POINTS);		
		for (int i = 0; i < m_SceneCld.GetSize(); i++)
		{
			glColor3f(m_SceneCld[i].color[0], m_SceneCld[i].color[1], m_SceneCld[i].color[2]);
			glVertex3f(m_SceneCld[i][0], m_SceneCld[i][1], m_SceneCld[i][2]);
		}
		glEnd();
	}
	if (m_planeList.size()>0)
	{
		for (int i = 0; i < m_planeList.size(); i++)
		{
			if (m_planeList[i].m_Vertexs.size() != 4)
				continue;

			glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_QUADS);
			glVertex3f(m_planeList[i].m_Vertexs[0][0], m_planeList[i].m_Vertexs[0][1], m_planeList[i].m_Vertexs[0][2]);
			glVertex3f(m_planeList[i].m_Vertexs[1][0], m_planeList[i].m_Vertexs[1][1], m_planeList[i].m_Vertexs[1][2]);
			glVertex3f(m_planeList[i].m_Vertexs[2][0], m_planeList[i].m_Vertexs[2][1], m_planeList[i].m_Vertexs[2][2]);
			glVertex3f(m_planeList[i].m_Vertexs[3][0], m_planeList[i].m_Vertexs[3][1], m_planeList[i].m_Vertexs[3][2]);
			glEnd();
		}		
	}
	if (m_targetBoxPnts.size()>0)
	{
		for (int i = 0; i < m_targetBoxPnts.size() / 2; i++)
		{
			GPoint minpt = m_targetBoxPnts[2 * i];
			GPoint maxpt = m_targetBoxPnts[2 * i + 1];
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP);
			glVertex3f(minpt[0], minpt[1], minpt[2]);
			glVertex3f(minpt[0], maxpt[1], minpt[2]);
			glVertex3f(maxpt[0], maxpt[1], minpt[2]);
			glVertex3f(maxpt[0], minpt[1], minpt[2]);
			glEnd();
			glBegin(GL_LINE_LOOP);
			glVertex3f(maxpt[0], maxpt[1], maxpt[2]);
			glVertex3f(minpt[0], maxpt[1], maxpt[2]);
			glVertex3f(minpt[0], minpt[1], maxpt[2]);
			glVertex3f(maxpt[0], minpt[1], maxpt[2]);
			glEnd();
			glBegin(GL_LINES);
			glVertex3f(minpt[0], minpt[1], minpt[2]);
			glVertex3f(minpt[0], minpt[1], maxpt[2]);
			glVertex3f(minpt[0], maxpt[1], minpt[2]);
			glVertex3f(minpt[0], maxpt[1], maxpt[2]);
			glVertex3f(maxpt[0], maxpt[1], minpt[2]);
			glVertex3f(maxpt[0], maxpt[1], maxpt[2]);
			glVertex3f(maxpt[0], minpt[1], minpt[2]);
			glVertex3f(maxpt[0], minpt[1], maxpt[2]);
			glEnd();
		}
	}

	glColor3f(1.0, 1.0, 1.0);
	glPopMatrix();
	glFinish();
	SwapBuffers(wglGetCurrentDC());
}

bool CPGDemoDlg::CreateFirstConnected()
{
	if (m_Kinect!=NULL)
	{
		_Write2LogS("kinect already connected");
		return true;
	}
	HRESULT hr = GetDefaultKinectSensor(&m_Kinect);
	if (FAILED(hr))
	{
		_Write2LogS("get default sensor failed");
		return false;
	}
	if (m_Kinect)
	{
		// Initialize the Kinect and get the color reader
		IColorFrameSource* pColorFrameSource = NULL;
		IDepthFrameSource* pDepthFrameSource = NULL;
		hr = m_Kinect->Open();
		if (SUCCEEDED(hr))
		{
			hr = m_Kinect->get_CoordinateMapper(&m_mapper);
		}
		if (SUCCEEDED(hr))
		{
			hr = m_Kinect->get_ColorFrameSource(&pColorFrameSource);
		}
		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_colorReader);
		}
		if (SUCCEEDED(hr))
		{
			hr = m_Kinect->get_DepthFrameSource(&pDepthFrameSource);
		}
		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameSource->OpenReader(&m_depthReader);
		}

		if (pColorFrameSource)
		{
			pColorFrameSource->Release();
			pColorFrameSource = NULL;
		}
		if (pDepthFrameSource)
		{
			pDepthFrameSource->Release();
			pDepthFrameSource = NULL;
		}
		if (FAILED(hr))
			return false;
		return true;
	}

	return false;
}

void CPGDemoDlg::OnBnClickedButtonConnect()
{
	if (CreateFirstConnected())
	{
		m_bKinectConnected = true;
		m_bKinectStreaming = true;
		m_hProcessThread = CreateThread(NULL, 0, ProcessThread, this, 0, NULL);

		((CButton*)GetDlgItem(IDC_BUTTON_CONNECT))->EnableWindow(false);
		((CButton*)GetDlgItem(IDC_BUTTON_DISCONNECT))->EnableWindow(true);
	}
}

DWORD WINAPI CPGDemoDlg::ProcessThread(LPVOID lpParam)
{
	CPGDemoDlg* p_this = reinterpret_cast<CPGDemoDlg*>(lpParam);
	return p_this->ProcessThread();
}

DWORD WINAPI CPGDemoDlg::ProcessThread()
{
	DWORD wait_mills = 0;
	while (true)
	{
		if (!m_bKinectConnected)
		{
			break;
		}

		if (!m_bKinectStreaming)
		{
			continue;
		}

		//_Write2LogS("start process");
		bool b_depth_grabbed = false;
		IColorFrame* color_frame = NULL;
		IDepthFrame* depth_frame = NULL;
		HRESULT hr;
		hr = m_colorReader->AcquireLatestFrame(&color_frame);
		if (SUCCEEDED(hr))
		{
			hr = m_depthReader->AcquireLatestFrame(&depth_frame);
		}
		if (SUCCEEDED(hr))
		{
			GetColorFrame(color_frame, &m_colorMat);
			b_depth_grabbed = GetDepthFrame(depth_frame, &m_depthMat16);
			if (b_depth_grabbed)
			{
				m_depthMat16.convertTo(m_depthMat, CV_8U);
				GetSceneCld(depth_frame, &m_SceneCld);
				if (((CButton*)GetDlgItem(IDC_CHECK_BACKGROUND_CALIBRATED))->GetCheck())
				{
					m_SceneCld.FilterPnts();
					m_config.LoadBkCalibration(m_bkTrans, m_bkRotAxis, m_bkRotRad);
					m_SceneCld.Translate(m_bkTrans);
					m_SceneCld.Rotate(GPoint(0, 0, 0), m_bkRotAxis, m_bkRotRad);
					if (((CButton*)GetDlgItem(IDC_CHECK_PROCESS))->GetCheck())
					{
						ResetValidFlags(m_inlierList);
						UpdateValidFlags(m_SceneCld.m_validList, FLAG_VALID_DATA);
						InitializeScene();
						ProcessScene();
					}
				}				
			}
			if (m_saveFrame)
			{
				CString ss;
				std::time_t t;
				std::time(&t);
				std::tm* data;
				data = std::localtime(&t);
				ss.Format("%s\\data\\%.2d%.2d%.2d%.2d", m_path, data->tm_mday, data->tm_hour, data->tm_min, data->tm_sec);
				cv::imwrite((ss + "_color.jpg").GetBuffer(), m_colorMat);
				cv::imwrite((ss + "_depth.jpg").GetBuffer(), m_depthMat);
				m_SceneCld.Save((ss + "_scene.asc").GetBuffer());
				m_saveFrame = false;
			}
		}
		SafeRelease(color_frame);
		SafeRelease(depth_frame);

		if (SUCCEEDED(hr))
			SendMessage(WM_PAINT);
		//_Write2LogS("end process");
		Sleep(10);
	}
	

	return 0;
}

bool CPGDemoDlg::GetColorFrame(IColorFrame* color, cv::Mat* frame)
{
	IFrameDescription* pFrameDescription = NULL;
	int img_width = 0;
	int img_height = 0;
	ColorImageFormat imageFormat = ColorImageFormat_None;
	UINT nBufferSize = 0;
	RGBQUAD *pBuffer = NULL;
	HRESULT hr = color->get_FrameDescription(&pFrameDescription);
	if (SUCCEEDED(hr))
	{
		hr = pFrameDescription->get_Width(&img_width);
	}
	if (SUCCEEDED(hr))
	{
		hr = pFrameDescription->get_Height(&img_height);
	}
	if (img_width != m_colorWidth || img_height != m_colorHeight)
	{
		_Write2LogS("color image resolution incorrect");
		return false;
	}
	if (SUCCEEDED(hr))
	{
		hr = color->get_RawColorImageFormat(&imageFormat);
	}
	if (SUCCEEDED(hr))
	{
		if (imageFormat == ColorImageFormat_Bgra)
		{
			hr = color->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
			_Write2LogS("color format 1");
		}
		else if (m_pColorRGBX)
		{
			pBuffer = m_pColorRGBX;
			nBufferSize = m_colorWidth * m_colorHeight * sizeof(RGBQUAD);
			hr = color->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
		}
		else
		{
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr))
	{
		uchar* p_mat = m_colorMat.data;

		const RGBQUAD* pBufferEnd = pBuffer + (m_colorWidth * m_colorHeight);

		while (pBuffer < pBufferEnd)
		{
			*p_mat = pBuffer->rgbBlue;
			p_mat++;
			*p_mat = pBuffer->rgbGreen;
			p_mat++;
			*p_mat = pBuffer->rgbRed;
			p_mat++;
			*p_mat = pBuffer->rgbReserved;
			p_mat++;

			++pBuffer;
		}

	}
	SafeRelease(pFrameDescription);
	return true;
}

bool CPGDemoDlg::GetDepthFrame(IDepthFrame* depth, cv::Mat* frame)
{
	IFrameDescription* pFrameDescription = NULL;
	int nWidth = 0;
	int nHeight = 0;
	UINT nBufferSize = 0;
	UINT16 *pBuffer = NULL;

	HRESULT hr = depth->get_FrameDescription(&pFrameDescription);
	if (SUCCEEDED(hr))
	{
		hr = pFrameDescription->get_Width(&nWidth);
	}
	if (SUCCEEDED(hr))
	{
		hr = pFrameDescription->get_Height(&nHeight);
	}
	if (SUCCEEDED(hr))
	{
		hr = depth->get_DepthMinReliableDistance(&m_minDepth);
	}
	if (SUCCEEDED(hr))
	{
		// In order to see the full range of depth (including the less reliable far field depth)
		// we are setting nDepthMaxDistance to the extreme potential depth threshold
		m_maxDepth = USHRT_MAX;

		// Note:  If you wish to filter by reliable depth distance, uncomment the following line.
		hr = depth->get_DepthMaxReliableDistance(&m_maxDepth);
	}
	if (SUCCEEDED(hr))
	{
		hr = depth->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
	}
	if (SUCCEEDED(hr))
	{
		for (UINT y = 0; y < m_depthHeight; ++y)
		{
			// get row pointer for depth mat
			//uchar* pdepthrow = frame->ptr<uchar>(y);
			//for (uint x = 0; x < m_depthWidth; ++x)
			//{
			//	UINT16 depth = *pBuffer;
			//	BYTE intensity = static_cast<BYTE>((depth >= m_minDepth) && (depth <= m_maxDepth) ? (depth % 256) : 0);
			//	pdepthrow[x] = intensity;
			//	pBuffer++;
			//}
			USHORT* pdepthrow = frame->ptr<USHORT>(y);
			for (UINT x = 0; x < m_depthWidth; x++)
			{
				UINT16 depth = *pBuffer;
				if (depth<m_minDepth || depth>m_maxDepth)
					depth = 0;
				pdepthrow[x] = depth;
				pBuffer++;
			}
		}
	}
	SafeRelease(pFrameDescription);
	return true;
}

void CPGDemoDlg::GetSceneCld(IDepthFrame* depth, GCloud* cld)
{
	UINT nBufferSize = 0;
	UINT16 *pBuffer = NULL;
	HRESULT hr = depth->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
	if (SUCCEEDED(hr))
	{
		for (int y = 0; y < m_depthHeight; y++){
			for (int x = 0; x < m_depthWidth; x++){

				DepthSpacePoint depthSpacePoint = { static_cast<float>(x), static_cast<float>(y) };
				UINT16 depth = *pBuffer;
				if (depth<m_minDepth || depth>m_maxDepth)
				{
					(*cld)[y*m_depthWidth + x][0] = 0;
					(*cld)[y*m_depthWidth + x][1] = 0;
					(*cld)[y*m_depthWidth + x][2] = 0;
				}
				else
				{
					// Coordinate Mapping Depth to Camera Space, and Setting PointCloud XYZ
					CameraSpacePoint cameraSpacePoint = { 0.0f, 0.0f, 0.0f };
					m_mapper->MapDepthPointToCameraSpace(depthSpacePoint, depth, &cameraSpacePoint);
					(*cld)[y*m_depthWidth + x][0] = cameraSpacePoint.X;
					(*cld)[y*m_depthWidth + x][1] = cameraSpacePoint.Y;
					(*cld)[y*m_depthWidth + x][2] = cameraSpacePoint.Z;
				}

				pBuffer++;
			}
		}
	}
}

void CPGDemoDlg::OnBnClickedButtonOpen()
{
	CFileDialog f_dialog(true, "asc", 0, 0, "ASC|*.asc|JPG|*.jpg||");
	if (IDOK == f_dialog.DoModal())
	{
		CString file_name = f_dialog.GetPathName();
		LoadScene(file_name);
		m_SceneCld.FilterPnts();
		//ResetValidFlags(m_SceneCld.m_validList);
		if (((CButton*)GetDlgItem(IDC_CHECK_BACKGROUND_CALIBRATED))->GetCheck())
		{
			m_config.LoadBkCalibration(m_bkTrans, m_bkRotAxis, m_bkRotRad);
			m_SceneCld.Translate(m_bkTrans);
			m_SceneCld.Rotate(GPoint(0, 0, 0), m_bkRotAxis, m_bkRotRad);
			LoadSegmentImg(m_path+"\\bk.jpg", m_inlierList);
			ResetValidFlags(m_inlierList);
			UpdateValidFlags(m_SceneCld.m_validList, FLAG_VALID_DATA);
		}
		SendMessage(WM_PAINT);
	}
}

void CPGDemoDlg::SaveScene()
{
	if (m_bKinectStreaming)
	{
		m_saveFrame = true;
	}
	else
	{
		CString ss;
		std::time_t t;
		std::time(&t);
		std::tm* data;
		data = std::localtime(&t);
		ss.Format("%s\\data\\%.2d%.2d%.2d%.2d", m_path, data->tm_mday, data->tm_hour, data->tm_min, data->tm_sec);

		cv::imwrite((ss + "_color.jpg").GetBuffer(), m_colorMat);
		cv::imwrite((ss + "_depth.jpg").GetBuffer(), m_depthMat);
		m_SceneCld.Save((ss + "_scene.asc").GetBuffer());
	}
	
}

void CPGDemoDlg::LoadScene(CString fn)
{
	CString ext = fn.Right(3);
	ext.MakeLower();
	if (ext == "jpg" || ext == "asc")
	{
		fn = fn.Left(fn.GetLength() - 10);
		m_colorMat = cv::imread((fn + "_color.jpg").GetBuffer());
		cv::cvtColor(m_colorMat, m_colorMat, CV_BGR2BGRA);
		m_depthMat = cv::imread((fn + "_depth.jpg").GetBuffer());
		m_SceneCld.Load((fn + "_scene.asc").GetBuffer());
		_Write2LogSI("load point num:", m_SceneCld.GetSize());
	}
}

void CPGDemoDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}

void CPGDemoDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bMouseLeftDown = true;
	m_MouseX = point.x;
	m_MouseY = point.y;

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CPGDemoDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bMouseLeftDown)
	{
		float x = point.x;
		float y = point.y;
		m_RotX += (y - m_MouseY) / 10.0f;
		m_RotY += (x - m_MouseX) / 10.0f;		
		m_MouseX = x;
		m_MouseY = y;
	}
	else if (m_bMouseRightDown)
	{
		float x = point.x;
		float y = point.y;
		m_TransX += (x - m_MouseX) / 100.0f;
		m_TransY += (y - m_MouseY) / 100.0f;
		m_MouseX = x;
		m_MouseY = y;		
	}
	if (!m_bKinectConnected)
		SendMessage(WM_PAINT);

	CDialogEx::OnMouseMove(nFlags, point);
}

BOOL CPGDemoDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (zDelta > 0)
		m_scale *= 0.85;
	else
		m_scale *= 1.15;
	if (!m_bKinectConnected)
		SendMessage(WM_PAINT);
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

void CPGDemoDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bMouseLeftDown = false;

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CPGDemoDlg::OnBnClickedButtonSave()
{
	SaveScene();
}


void CPGDemoDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_bMouseRightDown = true;
	m_MouseX = point.x;
	m_MouseY = point.y;

	CDialogEx::OnRButtonDown(nFlags, point);
}


void CPGDemoDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bMouseRightDown = false;

	CDialogEx::OnRButtonUp(nFlags, point);
}

void CPGDemoDlg::OnBnClickedButtonCalibrate()
{
	if (m_SceneCld.GetSize() == 0)
		return;

	GPoint plane_pnt, plane_nor;
	if (m_bKinectConnected)
		m_bKinectStreaming = false;
	m_SceneCld.FilterPnts();
	m_inlierList = m_SceneCld.Fit2PlaneRejection(plane_pnt, plane_nor);	
	SaveSegmentImg(m_path+"\\bk.jpg", m_inlierList);
	//GPlane new_plane(plane_pnt, plane_nor);
	//new_plane.UpdateVertexs();
	m_SceneCld.Translate(-plane_pnt);	
	//new_plane.Translate(-plane_pnt);
	GPoint rot_axis = plane_nor^GPoint(0, 0, 1);
	rot_axis.Normalize();
	double rot_rad = plane_nor.Angle2Vector(GPoint(0, 0, 1), rot_axis);
	m_SceneCld.Rotate(GPoint(0, 0, 0), rot_axis, rot_rad);

	m_config.SaveBkCalibration(-plane_pnt, rot_axis, rot_rad);
	if (m_bKinectConnected)
	{
		m_bKinectStreaming = true;
		((CButton*)GetDlgItem(IDC_CHECK_BACKGROUND_CALIBRATED))->SetCheck(true);
	}
	if (!m_bKinectConnected)
		SendMessage(WM_PAINT);
}

void CPGDemoDlg::SaveSegmentImg(CString fn, std::vector<int> ind_list)
{
	cv::Mat img(m_depthHeight, m_depthWidth, CV_8U, cv::Scalar(0));
	for (int i = 0; i < ind_list.size(); i++)
	{
		int row_no = ind_list[i] / m_depthWidth;
		int col_no = ind_list[i] % m_depthWidth;
		img.at<uchar>(row_no, col_no) = 255;
	}
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(img, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	int max_ind = 0;
	int max_num = contours[0].size();
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > max_num)
		{
			max_num = contours[i].size();
			max_ind = i;
		}
	}
	cv::Mat contour_img(img.size(), CV_8U, cv::Scalar(0));
	std::vector<std::vector<cv::Point>> filter_contours;
	filter_contours.push_back(contours[max_ind]);
	cv::drawContours(contour_img, filter_contours, -1, cv::Scalar(255), CV_FILLED);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(11, 11), cv::Point(5, 5));
	cv::erode(contour_img, contour_img, element);
	cv::imwrite(fn.GetBuffer(), contour_img);
	img.release();
	contour_img.release();
	element.release();
}

bool CPGDemoDlg::LoadSegmentImg(CString fn, std::vector<int>& ind_list)
{
	cv::Mat img = cv::imread(fn.GetBuffer(), CV_LOAD_IMAGE_GRAYSCALE);
	if (img.empty())
		return false;
	if (img.rows != m_depthHeight || img.cols != m_depthWidth)
		return false;
	ind_list.clear();
	for (int i = 0; i < m_depthHeight; i++)
	{
		uchar* data = img.ptr<uchar>(i);
		for (int j = 0; j < m_depthWidth; j++)
		{
			int intensity = *data;
			if (intensity>128)
				ind_list.push_back(i*m_depthWidth + j);
			data++;
		}
	}
	return true;
}


void CPGDemoDlg::OnBnClickedButtonTest()
{
	//std::vector<int> test_list;
	//LoadSegmentImg("c:\\tmp\\kinect\\bk.jpg", test_list);
	//SaveSegmentImg("c:\\tmp\\kinect\\bk1.jpg", test_list);
	//SaveDepth2ColorMapping();
	//PreprocessDepth2ColorMapping("c:\\tmp\\kinect\\depth2color.txt");
	m_config.LoadBkCalibration(m_bkTrans, m_bkRotAxis, m_bkRotRad);
	for (int i = 2; i < 32; i++)
	{
		CString ss;
		ss.Format("%s\\result\\bk_calibrated\\%d_scene.asc", m_path, i);
		GCloud cld;
		cld.Load(ss.GetString());
		cld.Rotate(GPoint(0, 0, 0), m_bkRotAxis, -m_bkRotRad);
		cld.Translate(-m_bkTrans);
		ss.Format("%s\\result\\%d_scene.asc", m_path, i);
		cld.Save(ss.GetBuffer());
	}
	
}

void CPGDemoDlg::UpdateValidFlags(std::vector<int> ind_list, int flag)
{
	if (!m_validFlags)
		return;
	int* flags = new int[m_depthWidth*m_depthHeight];
	memset(flags, 0, sizeof(int)*m_depthWidth*m_depthHeight);
	for (int i = 0; i < ind_list.size(); i++)
	{
		int row_no = ind_list[i] / m_depthWidth;
		int col_no = ind_list[i] % m_depthWidth;
		flags[row_no*m_depthWidth + col_no] = flag;
	}
	for (int i = 0; i < m_depthWidth*m_depthHeight; i++)
	{
		if (m_validFlags[i] && flags[i])
			m_validFlags[i] = flags[i];
	}
	delete[]flags;
}

void CPGDemoDlg::ResetValidFlags(std::vector<int> ind_list, int flag)
{
	if (!m_validFlags)
		return;
	memset(m_validFlags, 0, sizeof(int)*m_depthWidth*m_depthHeight);
	for (int i = 0; i < ind_list.size(); i++)
	{
		int row_no = ind_list[i] / m_depthWidth;
		int col_no = ind_list[i] % m_depthWidth;
		m_validFlags[row_no*m_depthWidth + col_no] = flag;
	}
}

void CPGDemoDlg::ShowScaleImage(CString wnd_name, cv::Mat* img, double scale)
{
	cv::Size dsize((int)(img->cols*scale), (int)(img->rows*scale));
	cv::Mat new_img(dsize, img->type());
	cv::resize(*img, new_img, dsize);
	cv::imshow(wnd_name.GetBuffer(), new_img);
}

void CPGDemoDlg::OnBnClickedButtonProcess()
{
	InitializeScene();
	ProcessScene();
}

void CPGDemoDlg::ProcessScene()
{
	double height_thres = 0.02;
	int num_thres = 20;
	double size_thres = 0.5;
	double box_size_thres = 0.95;
	//get potential top data of target
	std::vector<int> valid_index;
	for (int i = 0; i < m_depthWidth*m_depthHeight; i++)
	{
		if (m_validFlags[i])
		{
			if (m_SceneCld[i][2] > height_thres)
				valid_index.push_back(i);
		}
	}
	cv::Mat img(m_depthHeight, m_depthWidth, CV_8U, cv::Scalar(0));
	for (int i = 0; i < valid_index.size(); i++)
	{
		int row_no = valid_index[i] / m_depthWidth;
		int col_no = valid_index[i] % m_depthWidth;
		img.at<uchar>(row_no, col_no) = 255;
	}
	//cv::imwrite("c:\\tmp\\kinect\\segment.jpg", img);
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(img, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	if (contours.size() == 0)
		return;
	std::vector<std::vector<cv::Point>> filter_contours;
	//_Write2LogSI("contour num before filter:", contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		if (contours[i].size() > num_thres)
		{
			filter_contours.push_back(contours[i]);
		}
	}
	if (filter_contours.size() == 0)
		return;
	cv::Mat result(img.size(), CV_8U, cv::Scalar(0));
	cv::drawContours(result, filter_contours, -1, cv::Scalar(255), CV_FILLED);
	_Write2LogSI("contour num after filter:", filter_contours.size());
	for (int i = 0; i < filter_contours.size(); i++)
	{
		_Write2LogSI("process contour", i);
		GTarget target;
		GCloud cld;
		cv::Mat proj(img.size(), CV_8U, cv::Scalar(0));
		cv::Rect rect = cv::boundingRect(cv::Mat(filter_contours[i]));
		CString txt;
		int* blob_flag = new int[m_depthWidth*m_depthHeight];
		memset(blob_flag, 0, sizeof(int)*m_depthWidth*m_depthHeight);
		for (int j = rect.y; j < rect.y + rect.height; j++)
		{
			uchar* ptr = result.ptr<uchar>(j);
			for (int k = rect.x; k < rect.x + rect.width; k++)
			{
				int intensity = ptr[k];
				if (intensity == 255)
					blob_flag[j*m_depthWidth + k] = 1;
			}
		}
		std::vector<int> index_list;
		for (int j = rect.y; j < rect.y + rect.height; j++)
		{
			for (int k = rect.x; k < rect.x + rect.width; k++)
			{
				if (blob_flag[j*m_depthWidth + k] && m_validFlags[j*m_depthWidth + k] == FLAG_VALID_DATA)
				{
					cld.AddPoint(m_SceneCld[j*m_depthWidth + k]);
					m_SceneCld[j*m_depthWidth + k].SetColor(m_colorList[i%m_colorList.size()]);
					index_list.push_back(j*m_depthWidth + k);
				}
			}
		}
		//expand search area, find potential points close to contour
		//reflective and transparent botton cap may cause invalid data, which may be included as well
		int strx = rect.x - rect.width / 2;
		int endx = rect.x + rect.width + rect.width / 2;
		int stry = rect.y - rect.height / 2;
		int endy = rect.y + rect.height + rect.height / 2;
		for (int j = stry; j <= endy; j++)
		{
			for (int k = strx; k <= endx; k++)
			{
				if (j < 0 || j >= m_depthHeight || k < 0 || k >= m_depthWidth)
					continue;
				if (blob_flag[j*m_depthWidth + k])
					proj.at<uchar>(j, k) = 255;
				else if (m_validFlags[j*m_depthWidth + k] == FLAG_VALID_BACKGROUND)
					proj.at<uchar>(j, k) = 255;
			}
		}
		//after adding potential valid poitn, try to find contours again
		std::vector<std::vector<cv::Point>> proj_contours;
		cv::findContours(proj, proj_contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		//cv::Mat proj_contour(img.size(), CV_8U, cv::Scalar(0));
		//cv::drawContours(proj_contour, proj_contours, -1, cv::Scalar(255), CV_FILLED);
		//txt.Format("c:\\tmp\\kinect\\contour_%d.jpg", i);
		//cv::imwrite(txt.GetBuffer(), proj_contour);
		proj.release();
		delete[]blob_flag;
		if (proj_contours.size() == 0)
		{
			_Write2LogS("contour size equal to 0");
			continue;
		}
		//find the maximum contour to represent the bottle
		int ci = 0;
		float area = cv::contourArea(proj_contours[ci]);
		if (proj_contours.size() > 1)
		{
			for (int m = 1; m < proj_contours.size(); m++)
			{
				float new_area = cv::contourArea(proj_contours[m]);
				if (new_area > area)
				{
					area = new_area;
					ci = m;
				}
			}
		}		
		cv::RotatedRect rot_rect = cv::minAreaRect(cv::Mat(proj_contours[ci]));
		cv::Point2f vertices[4];
		rot_rect.points(vertices);
		float rect_ratio = area / (rot_rect.size.width*rot_rect.size.height);
		_Write2LogSF("rect ratio=", rect_ratio);
		if (rect_ratio > box_size_thres)
		{
			//box
			target.m_type = TARGET_BOX;		
			GPoint cen_pnt(rot_rect.center.x, rot_rect.center.y, 0.0f);
			target.m_center = cen_pnt;
			for (int m = 0; m < 4; m++)
				target.m_vertexs.push_back(GPoint(vertices[m].x, vertices[m].y, 0.0f));
		}
		else
		{
			float radius;
			cv::Point2f center;
			cv::minEnclosingCircle(cv::Mat(proj_contours[ci]), center, radius);
			//cv::circle(proj_contour, cv::Point(center), static_cast<int>(radius), cv::Scalar(128));
			float circle_ratio = area / (PI*radius*radius);
			_Write2LogSF("circle ratio=", circle_ratio);
			if (circle_ratio < size_thres)
			{
				_Write2LogS("contour size not meet size limit");
				continue;
			}
			cv::RotatedRect fit_ellipse = cv::fitEllipse(cv::Mat(proj_contours[ci]));
			target.m_type = TARGET_BOTTLE;
			GPoint cen_pnt(fit_ellipse.center.x, fit_ellipse.center.y, 0.0f);
			target.m_center = cen_pnt;
			fit_ellipse.points(vertices);
			//for (int m = 0; m<4; m++)
			//{
			//	cv::line(proj_contour, vertices[m], vertices[(m + 1) % 4], cv::Scalar(128));    
			//}
			for (int m = 0; m < 4; m++)
				target.m_vertexs.push_back(GPoint(vertices[m].x, vertices[m].y, 0.0f));
		}
		//cv::imwrite(txt.GetBuffer(), proj_contour);
		//proj_contour.release();
		cld.UpdateBndBox();
		cld.m_boxMinPnt[2] = 0;//set to zero
		if (cld.m_boxMaxPnt[2] < height_thres)
		{
			_Write2LogS("height below limit");
			continue;
		}
		m_targetBoxPnts.push_back(cld.m_boxMinPnt);
		m_targetBoxPnts.push_back(cld.m_boxMaxPnt);
		target.m_cld = cld;
		target.m_indexList = index_list;
		if (PaintTargetInColor(target))
		{
			m_targets.push_back(target);
			if (m_saveResult)
			{
				CString ss, txt;
				GetDlgItemText(IDC_EDIT_SAVENAME, txt);
				ss.Format("%s\\result\\%s", m_path, txt);
				cv::imwrite((ss + "_color.jpg").GetBuffer(), m_colorMat);
				m_SceneCld.Save((ss + "_scene.asc").GetBuffer());
				m_saveResult = false;
			}
		}
	}
	//cv::imwrite("c:\\tmp\\kinect\\contours.jpg", result);
	_Write2LogSI("extract target number:", m_targets.size());
	img.release();
	result.release();
	if (!m_bKinectStreaming)
		SendMessage(WM_PAINT);
}

void CPGDemoDlg::InitializeScene()
{
	m_targets.clear();
	m_targetBoxPnts.clear();
	for (int i = 0; i < m_SceneCld.GetSize(); i++)
		m_SceneCld[i].SetColor(GPoint(1.0, 1.0, 1.0));
}

bool CPGDemoDlg::LoadRefTargetList()
{
	if (m_refTargets.size())
	{
		m_refTargets.clear();
		for (int i = 0; i < m_refImgList.size(); i++)
		{
			m_refImgList[i].release();
			m_refImgList[i] = NULL;
		}
		m_refImgList.clear();
	}
	CString txt;
	txt.Format("%s\\pg_sample_list.txt", m_refDirectory);
	CStdioFile sfile;
	if (sfile.Open(txt, CStdioFile::modeRead))
	{
		
		while (sfile.ReadString(txt))
		{
			GTarget tar;
			if (sscanf(txt, "%d\t%d\t%f\t%f\t%f\t%f\t%f\n", &tar.m_sampleNo, &tar.m_type, &tar.m_majorRadius, &tar.m_minorRadius, &tar.m_height, &tar.m_length, &tar.m_width) == 7)
			{
				tar.m_majorRadius /= 2.0;
				tar.m_minorRadius /= 2.0;
				m_refTargets.push_back(tar);
				txt.Format("%s\\%d.jpg", m_refDirectory, tar.m_sampleNo);
				cv::Mat sample_img;
				sample_img = cv::imread(txt.GetBuffer());
				if (sample_img.empty())
				{
					sample_img.create(100, 100, CV_8UC4);
				}
				m_refImgList.push_back(sample_img);
			}
		}
		sfile.Close();
		return true;
	}
	else
		return false;

}

int CPGDemoDlg::RecognizeTarget(GTarget tar)
{
	CString txt;
	txt.Format("new target, major radius=%f, minor radius=%f, height=%f", tar.m_majorRadius, tar.m_minorRadius, tar.m_height);
	_Write2LogS(txt.GetString());
	int tar_index = -1;
	if (tar.m_type == TARGET_BOTTLE)
	{
		std::vector<int> candidate_list;
		double radius_diff_thres = 0.01;
		//double height_diff_thres = 0.005;
		double delta_threshold = 0.01;//10mm
		for (int i = 0; i < m_refTargets.size(); i++)
		{
			if (m_refTargets[i].m_type != TARGET_BOTTLE)
				continue;
			if (fabs(m_refTargets[i].m_majorRadius - tar.m_majorRadius) < radius_diff_thres &&
				fabs(m_refTargets[i].m_minorRadius - tar.m_minorRadius) < radius_diff_thres)
				candidate_list.push_back(i);
			//if (fabs(m_refTargets[i].m_height - tar.m_height) < height_diff_thres)
			//	candidate_list.push_back(i);
			//candidate_list.push_back(i);
		}

		double coe_length = 0.2;
		double coe_width = 0.2;
		double coe_height = 0.6;
		double min_delta;

		for (int i = 0; i < candidate_list.size(); i++)
		{
			double delta = coe_length*fabs(m_refTargets[candidate_list[i]].m_majorRadius - tar.m_majorRadius) +
				coe_width*fabs(m_refTargets[candidate_list[i]].m_minorRadius - tar.m_minorRadius) +
				coe_height*fabs(m_refTargets[candidate_list[i]].m_height - tar.m_height);
			if (tar_index < 0)
			{
				tar_index = candidate_list[i];
				min_delta = delta;
			}
			else if (delta < min_delta)
			{
				tar_index = candidate_list[i];
				min_delta = delta;
			}
		}

		if (tar_index == -1)
		{
			_Write2LogS("target not recognized");
			return tar_index;
		}
		txt.Format("target recongnition results, minimum delta=%f, minimum target index=%d", min_delta, m_refTargets[tar_index].m_sampleNo);
		_Write2LogS(txt.GetString());
		if (min_delta>delta_threshold)
			tar_index = -1;
	}

	return tar_index;
}

bool CPGDemoDlg::PaintTargetInColor(GTarget& tar)
{
	_Write2LogS("paint new target in color frame");
	CString txt;
	if (tar.m_type == TARGET_BOTTLE)
	{
		//bottle
		int cx, cy, radius;
		GPoint scene_cen_pt, scene_major_pt, scene_minor_pt;
		GPoint cen_pt(tar.m_center[0], tar.m_center[1], 0.0f);
		GPoint major_pt((tar.m_vertexs[0][0] + tar.m_vertexs[1][0]) / 2, (tar.m_vertexs[0][1] + tar.m_vertexs[1][1]) / 2, 0.0f);
		GPoint minor_pt((tar.m_vertexs[1][0] + tar.m_vertexs[2][0]) / 2, (tar.m_vertexs[1][1] + tar.m_vertexs[2][1]) / 2, 0.0f);
		if (m_bKinectStreaming)
		{
			//_Write2LogS("online transform depth to color");
			//center point
			DepthSpacePoint depth_cen_pt = { cen_pt[0], cen_pt[1] };
			ColorSpacePoint color_cen_pt = { 0.0f, 0.0f };
			USHORT depth16 = m_depthMat16.at<USHORT>(static_cast<int>(depth_cen_pt.Y), static_cast<int>(depth_cen_pt.X));
			m_mapper->MapDepthPointToColorSpace(depth_cen_pt, depth16, &color_cen_pt);

			//major raidus end
			DepthSpacePoint depth_major_pt = { major_pt[0], major_pt[1] };
			ColorSpacePoint color_major_pt = { 0.0f, 0.0f };
			depth16 = m_depthMat16.at<USHORT>(static_cast<int>(depth_major_pt.Y), static_cast<int>(depth_major_pt.X));
			m_mapper->MapDepthPointToColorSpace(depth_major_pt, depth16, &color_major_pt);

			//minor radius end
			DepthSpacePoint depth_minor_pt = { minor_pt[0], minor_pt[1] };
			ColorSpacePoint color_minor_pt = { 0.0f, 0.0f };
			depth16 = m_depthMat16.at<USHORT>(static_cast<int>(depth_minor_pt.Y), static_cast<int>(depth_minor_pt.X));
			m_mapper->MapDepthPointToColorSpace(depth_minor_pt, depth16, &color_minor_pt);
			cx = static_cast<int>(color_cen_pt.X);
			cy = static_cast<int>(color_cen_pt.Y);
			//_Write2LogSI("cx=", cx);
			//_Write2LogSI("cy=", cy);
			int radius1, radius2;
			radius1 = static_cast<int>(sqrt((color_major_pt.X - color_cen_pt.X)*(color_major_pt.X - color_cen_pt.X) + (color_major_pt.Y - color_cen_pt.Y)*(color_major_pt.Y - color_cen_pt.Y)));
			radius2 = static_cast<int>(sqrt((color_minor_pt.X - color_cen_pt.X)*(color_minor_pt.X - color_cen_pt.X) + (color_minor_pt.Y - color_cen_pt.Y)*(color_minor_pt.Y - color_cen_pt.Y)));
			radius = radius1 > radius2 ? radius1 : radius2;
			scene_cen_pt = m_SceneCld[static_cast<int>(cen_pt[1])*m_depthWidth + static_cast<int>(cen_pt[0])];
			scene_major_pt = m_SceneCld[static_cast<int>(major_pt[1])*m_depthWidth + static_cast<int>(major_pt[0])];
			scene_minor_pt = m_SceneCld[static_cast<int>(minor_pt[1])*m_depthWidth + static_cast<int>(minor_pt[0])];
		}
		else
		{
			//offline, kinect not connected
			GPoint color_cen_pt = ConvertPntDepth2ColorStatic(cen_pt);
			GPoint color_major_pt = ConvertPntDepth2ColorStatic(major_pt);
			GPoint color_minor_pt = ConvertPntDepth2ColorStatic(minor_pt);
			cx = static_cast<int>(color_cen_pt[0]);
			cy = static_cast<int>(color_cen_pt[1]);
			int radius1, radius2;
			radius1 = static_cast<int>((color_major_pt - color_cen_pt).Length2D());
			radius2 = static_cast<int>((color_minor_pt - color_cen_pt).Length2D());
			radius = radius1 > radius2 ? radius1 : radius2;
			scene_cen_pt = m_SceneCld[static_cast<int>(cen_pt[1])*m_depthWidth + static_cast<int>(cen_pt[0])];
			scene_major_pt = m_SceneCld[static_cast<int>(major_pt[1])*m_depthWidth + static_cast<int>(major_pt[0])];
			scene_minor_pt = m_SceneCld[static_cast<int>(minor_pt[1])*m_depthWidth + static_cast<int>(minor_pt[0])];
		}
		if (cx < 0 || cy < 0 || cx >= m_colorWidth || cy >= m_colorHeight)
			return false;
		cv::circle(m_colorMat, cv::Point(cx, cy), static_cast<int>(radius), cv::Scalar(0, 255, 0));
		tar.m_majorRadius = (scene_major_pt - scene_cen_pt).Length2D();
		tar.m_minorRadius = (scene_minor_pt - scene_cen_pt).Length2D();
		if (tar.m_majorRadius < tar.m_minorRadius)
		{
			float tmp_float = tar.m_majorRadius;
			tar.m_majorRadius = tar.m_minorRadius;
			tar.m_minorRadius = tmp_float;
		}
		//tar.m_height = tar.m_cld.GetMassCenter()[2];
		tar.m_height = tar.m_cld.m_boxMaxPnt[2];
		txt.Format("ra=%.3f, ri=%.3f, h=%.3f", tar.m_majorRadius, tar.m_minorRadius, tar.m_height);
		cv::putText(m_colorMat, txt.GetBuffer(), cv::Point(cx+radius, cy-radius), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(0,0,255),1);
		int ref_target_index = RecognizeTarget(tar);
		//_Write2LogSI("recognized target no:", ref_target_index);
		if (ref_target_index > -1)
		{
			txt.Format("sample #%d", m_refTargets[ref_target_index].m_sampleNo);
			cv::putText(m_colorMat, txt.GetBuffer(), cv::Point(cx + radius, cy - radius+20), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255), 1);
			_Write2LogSI("target recognized:", m_refTargets[ref_target_index].m_sampleNo);
			//txt.Format("%s\\%d.jpg", m_refDirectory, ref_target_index + 1);
			PaintSubImg(ref_target_index, static_cast<int>(cx + radius), static_cast<int>(cy - radius) + 40);
		}
	}
	else
	{
		//box
		if (m_bKinectStreaming)
		{

		}
		else
		{
			std::vector<GPoint> c_vertexs;
			for (int i = 0; i < tar.m_vertexs.size(); i++)
			{
				c_vertexs.push_back(ConvertPntDepth2ColorStatic(tar.m_vertexs[i]));
			}
			if (c_vertexs.size() == 4)
			{
				for (int m = 0; m<4; m++)
				{
					cv::line(m_colorMat, cv::Point2f(c_vertexs[m][0], c_vertexs[m][1]), cv::Point2f(c_vertexs[(m + 1) % 4][0], c_vertexs[(m + 1) % 4][1]), cv::Scalar(0, 255, 0));
				}
			}
		}
	}
	return true;
}

GPoint CPGDemoDlg::ConvertPntDepth2ColorStatic(GPoint pt)
{
	GPoint cpt;
	if (m_depth2ColorMapper.size() == 0)
	{
		CStdioFile map_file;
		if (map_file.Open(m_path+"\\depth2color.txt", CStdioFile::modeRead))
		{
			CString txt;
			int x, y;
			while (map_file.ReadString(txt))
			{
				if (sscanf(txt, "%d,%d", &x, &y) == 2)
				{
					GPoint pnt(x, y, 0);
					m_depth2ColorMapper.push_back(pnt);
				}
			}
			map_file.Close();

			if (m_depth2ColorMapper.size() == m_depthWidth*m_depthHeight)
			{
				x = static_cast<int>(pt[0]);
				y = static_cast<int>(pt[1]);
				cpt = m_depth2ColorMapper[y*m_depthWidth + x];
			}
			else
			{
				m_depth2ColorMapper.clear();
				cpt[0] = pt[0] / m_depthWidth*m_colorWidth;
				cpt[1] = pt[1] / m_depthHeight*m_colorHeight;
				cpt[2] = 0;
			}
		}
		else
		{
			cpt[0] = pt[0] / m_depthWidth*m_colorWidth;
			cpt[1] = pt[1] / m_depthHeight*m_colorHeight;
			cpt[2] = 0;
		}
	}
	else
	{
		int x, y;
		x = static_cast<int>(pt[0]);
		y = static_cast<int>(pt[1]);
		cpt = m_depth2ColorMapper[y*m_depthWidth + x];
	}
	
	if (cpt[0] < 0 || cpt[0] >= m_colorWidth)
		cpt[0] = 0;
	if (cpt[1] < 0 || cpt[1] >= m_colorHeight)
		cpt[1] = 0;

	return cpt;
}

void CPGDemoDlg::PaintSubImg(CString img_name, int px, int py)
{
	cv::Mat sub_img = cv::imread(img_name.GetBuffer());
	if (sub_img.empty())
		return;
	cv::cvtColor(sub_img, sub_img, CV_BGR2BGRA);
	int sub_type = sub_img.channels();
	int color_type = m_colorMat.channels();
	int sub_width = sub_img.size().width;
	int sub_height = sub_img.size().height;
	for (int j = 0; j < sub_height; j++)
	{
		if (py + j < 0 || py + j >= m_colorHeight)
			continue;
		cv::Vec4b* img_ptr = m_colorMat.ptr<cv::Vec4b>(py + j);
		cv::Vec4b* sub_ptr = sub_img.ptr<cv::Vec4b>(j);
		for (int i = 0; i < sub_width; i++)
		{
			if (px + i < 0 || px + i >= m_colorWidth)
				continue;
			img_ptr[i + px] = sub_ptr[i];
		}
	}
	sub_img.release();
}

void CPGDemoDlg::PaintSubImg(int ref_no, int px, int py)
{
	if (ref_no>m_refImgList.size() - 1)
		return;
	cv::Mat sub_img;
	cv::cvtColor(m_refImgList[ref_no], sub_img, CV_BGR2BGRA);
	int sub_type = sub_img.channels();
	int color_type = m_colorMat.channels();
	int sub_width = sub_img.size().width;
	int sub_height = sub_img.size().height;
	for (int j = 0; j < sub_height; j++)
	{
		if (py + j < 0 || py + j >= m_colorHeight)
			continue;
		cv::Vec4b* img_ptr = m_colorMat.ptr<cv::Vec4b>(py + j);
		cv::Vec4b* sub_ptr = sub_img.ptr<cv::Vec4b>(j);
		for (int i = 0; i < sub_width; i++)
		{
			if (px + i < 0 || px + i >= m_colorWidth)
				continue;
			img_ptr[i + px] = sub_ptr[i];
		}
	}
	sub_img.release();
}

void CPGDemoDlg::SaveDepth2ColorMapping()
{
	if (!m_bKinectStreaming)
		return;
	CStdioFile map_file;
	CString fname;
	fname.Format("%s\\depth2color.txt", m_path);
	if (map_file.Open(fname, CStdioFile::modeCreate | CStdioFile::modeWrite))
	{
		CString txt;
		m_bKinectStreaming = false;
		for (int j = 0; j < m_depthHeight; j++)
		{
			for (int i = 0; i < m_depthWidth; i++)
			{
				DepthSpacePoint depth_pt = { i, j };
				ColorSpacePoint color_pt = { 0.0f, 0.0f };
				USHORT depth16 = m_depthMat16.at<USHORT>(j, i);
				m_mapper->MapDepthPointToColorSpace(depth_pt, depth16, &color_pt);
				txt.Format("%d,%d\n", static_cast<int>(color_pt.X), static_cast<int>(color_pt.Y));
				map_file.WriteString(txt);
			}
		}
		map_file.Close();
		m_bKinectStreaming = true;
	}
}

void CPGDemoDlg::PreprocessDepth2ColorMapping(CString fpath)
{
	CStdioFile map_file;
	if (map_file.Open(fpath, CStdioFile::modeRead))
	{
		CString txt;
		int x, y;
		std::vector<GPoint> pnt_list;
		int* flags = new int[m_depthWidth*m_depthHeight];
		memset(flags, 0, sizeof(int)*m_depthWidth*m_depthHeight);
		while (map_file.ReadString(txt))
		{
			if (sscanf(txt, "%d,%d", &x, &y) == 2)
			{
				GPoint pnt(x, y, 0);
				pnt_list.push_back(pnt);
			}
		}
		map_file.Close();
		if (pnt_list.size() != m_depthWidth*m_depthHeight)
		{
			_Write2LogS("inconsistant pixel number");
			return;
		}
		for (int i = 0; i < pnt_list.size(); i++)
		{
			GPoint pt = pnt_list[i];
			if (pt[0] < 0 || pt[1] < 0 || pt[0] >= m_colorWidth || pt[1] >= m_colorHeight)
			{
				flags[i] = 1;
			}				
		}
		while (true)
		{
			int invalid_num_before = 0;
			for (int i = 0; i < pnt_list.size(); i++)
			{
				if (flags[i])
					invalid_num_before++;
			}
			_Write2LogSI("invalid number before,", invalid_num_before);
			for (int i = 0; i < m_depthHeight; i++)
			{
				for (int j = 0; j < m_depthWidth; j++)
				{
					if (flags[i*m_depthWidth + j])
					{
						float valid_count = 0.0;
						float x_sum = 0.0;
						float y_sum = 0.0;
						for (int m = i - 1; m <= i + 1; m++)
						{
							for (int n = j - 1; n <= j + 1; n++)
							{
								if (m < 0 || n < 0 || m >= m_depthHeight || n >= m_depthWidth)
									continue;
								if (!flags[m*m_depthWidth + n])
								{
									//valid value
									GPoint pt = pnt_list[m*m_depthWidth + n];
									x_sum += pt[0];
									y_sum += pt[1];
									valid_count += 1.0;
								}
							}
						}
						if (valid_count > 0)
						{
							pnt_list[i*m_depthWidth + j][0] = x_sum / valid_count;
							pnt_list[i*m_depthWidth + j][1] = y_sum / valid_count;
							flags[i*m_depthWidth + j] = 0;
						}
					}
				}
			}
			int invalid_num_after = 0;
			for (int i = 0; i < pnt_list.size(); i++)
			{
				if (flags[i])
					invalid_num_after++;
			}
			_Write2LogSI("invalid number after,", invalid_num_after);
			if (invalid_num_after == 0)
				break;
			if (invalid_num_after == invalid_num_before)
				break;
		}
		delete[]flags;
		//save
		if (map_file.Open(fpath, CStdioFile::modeWrite | CStdioFile::modeCreate))
		{
			for (int i = 0; i < pnt_list.size(); i++)
			{
				txt.Format("%d,%d\n", static_cast<int>(pnt_list[i][0]), static_cast<int>(pnt_list[i][1]));
				map_file.WriteString(txt);
			}
			map_file.Close();
		}
	}
}

void CPGDemoDlg::OnBnClickedButtonRestoreview()
{
	m_RotX = 0;
	m_RotY = 0;
	m_TransX = 0;
	m_TransY = 0;
	m_scale = 1;
	if (!m_bKinectConnected)
		SendMessage(WM_PAINT);
}


void CPGDemoDlg::OnBnClickedButtonRecognitionresult()
{
	m_saveResult = true;
}


void CPGDemoDlg::OnBnClickedButtonDisconnect()
{
	if (!m_bKinectConnected)
		return;

	m_bKinectConnected = false;

	if (m_Kinect)
	{
		m_Kinect->Close();
		m_Kinect = NULL;
	}

	if (m_hProcessThread)
	{
		//WaitForSingleObject(m_hProcessThread, INFINITE);
		CloseHandle(m_hProcessThread);
		m_hProcessThread = NULL;
	}

	
	m_bKinectStreaming = false;

	((CButton*)GetDlgItem(IDC_BUTTON_CONNECT))->EnableWindow(true);
	((CButton*)GetDlgItem(IDC_BUTTON_DISCONNECT))->EnableWindow(false);
}


void CPGDemoDlg::OnBnClickedCheckProcess()
{
	if (!m_bKinectConnected)
		return;

	if (((CButton*)GetDlgItem(IDC_CHECK_PROCESS))->GetCheck())
	{
		if (!((CButton*)GetDlgItem(IDC_CHECK_BACKGROUND_CALIBRATED))->GetCheck())
		{
			AfxMessageBox("Background has to be calibrated before processing");
			((CButton*)GetDlgItem(IDC_CHECK_PROCESS))->SetCheck(false);
		}
	}
}


void CPGDemoDlg::OnBnClickedCancel()
{
	m_bKinectStreaming = false;
	cv::destroyAllWindows();
	
	if (m_validFlags)
		delete[]m_validFlags;

	if (m_pColorRGBX)
	{
		delete[]m_pColorRGBX;
		m_pColorRGBX = NULL;
	}
	if (m_pDepthRGBX)
	{
		delete[]m_pDepthRGBX;
		m_pDepthRGBX = NULL;
	}

	SafeRelease(m_colorReader);
	SafeRelease(m_depthReader);

	if (m_Kinect)
	{
		m_Kinect->Close();
		m_Kinect = NULL;
	}

	if (m_hProcessThread)
	{
		//WaitForSingleObject(m_hProcessThread, INFINITE);
		CloseHandle(m_hProcessThread);
		m_hProcessThread = NULL;
	}

	m_colorMat.release();
	m_depthMat.release();
	
	if (m_dc)
	{
		DeleteDC(m_dc);
	}

	CDialogEx::OnCancel();
}
