#include "camerathread.h"
#include "tchar.h"
#include "imageconverter.h"
#include <iostream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#define BUFFER_COUNT (64)

CameraThread::CameraThread(ImageChannelPtr pImageChannel)
	: mpImageChannel(pImageChannel)
{
	// TODO:initialize
	mIsCapturing = false;

	
}

CameraThread::~CameraThread()
{
	mIsCapturing = false;
	delete mpDevice;
	delete mpStream;
	delete mpPipeline;
}

int CameraThread::ConnectToDevice()
{
	PvDeviceFinderWnd finderWindow;
	PvResult aResult = finderWindow.ShowModal();

	if (!aResult.IsOK() || (finderWindow.GetSelected() == nullptr))
	{
		return -1;
	}

	const PvDeviceInfo *pDeviceInfo = finderWindow.GetSelected();

	if (pDeviceInfo == nullptr)
	{
		std::cout << "no device info" << std::endl;
		return -1;
	}
		
	Disconnect();

	PvResult result = PvResult::Code::NOT_CONNECTED;
	mpDevice = PvDevice::CreateAndConnect(pDeviceInfo, &result);
	if (!result.IsOK())
	{
		Disconnect();
		return -1;
	}

	mpStream = PvStream::CreateAndOpen(pDeviceInfo->GetConnectionID(), &result);
	if (!result.IsOK())
	{
		Disconnect();
		return -1;
	}

	PvGenParameterArray* pDeviceParams = mpDevice->GetParameters();
	mpStartCommand = dynamic_cast<PvGenCommand*>(pDeviceParams->Get("AcquisitionStart"));
	mpStopCommand = dynamic_cast<PvGenCommand*>(pDeviceParams->Get("AcquisitionStop"));

	if (mpPipeline != nullptr)
		delete mpPipeline;
	mpPipeline = new PvPipeline(mpStream);

	uint32_t size = mpDevice->GetPayloadSize();

	mpPipeline->SetBufferCount(BUFFER_COUNT);
	mpPipeline->SetBufferSize(size);

	return 0;
}

void CameraThread::Disconnect()
{
	if (mpDevice != nullptr)
	{
		PvDevice::Free(mpDevice);
		mpDevice = nullptr;
	}

	if (mpStream != nullptr)
	{
		PvStream::Free(mpStream);
		mpStream = nullptr;
	}
}

void CameraThread::StartCapture()
{
	mIsCapturing = true;
	std::thread captureThread = std::thread(&CameraThread::CaptureImage, this);
	captureThread.detach();
}

void CameraThread::StopCapture()
{
	mIsCapturing = false;
}

void CameraThread::CaptureImage()
{
	//初始化采集
	mpPipeline->Start();
	mpDevice->StreamEnable();
	mpStartCommand->Execute();
	
	while (mIsCapturing)
	{
		//TODO:后台线程检测
		//if (!mCaptureDevice->IsConnected()) {
		//	//StopCaptureThread();
		//	emit connectLost();
		//	break;
		//}
		PvBuffer* pBuffer = nullptr;
		PvResult result = mpPipeline->RetrieveNextBuffer(&pBuffer, 1000, nullptr);
		if (!result.IsOK()) {
			continue;
		}
		if (!pBuffer) {
			continue;
		}
		unsigned char *img_ptr = pBuffer->GetDataPointer();
		mpPipeline->ReleaseBuffer(pBuffer);
		QImage qImg = ImageConverter::Rawdata2QImage(img_ptr, std::stoi(mCameraParams.imageWidth), std::stoi(mCameraParams.imageHeight), QImage::Format_Indexed8);
		
		std::cout << "capture one image" << std::endl;
		//一帧图像放入队列
		mpImageChannel->PushImage(qImg);
		
	}
	mpStopCommand->Execute();
	mpDevice->StreamDisable();
	mpPipeline->Stop();
}

bool CameraThread::IsCapturing()
{
	return mIsCapturing == true;
}

void CameraThread::WriteParameter(QString cmd, const HWND &child, const HWND &send2)
{
	SetWindowTextW(child, (const wchar_t*)cmd.utf16());
	SendMessage(send2, WM_LBUTTONDOWN, 0, 0);
	SendMessage(send2, WM_LBUTTONUP, 0, 0);
	Sleep(100);
}

void CameraThread::SetExternSynchro(const HWND &child, const HWND &send2)
{
	WriteParameter("#S(1)", child, send2);
}
void CameraThread::SetInternSynchro(const HWND &child, const HWND &send2)
{
	WriteParameter("#S(0)", child, send2);
}
void CameraThread::SetPositiveEdge(const HWND &child, const HWND &send2)
{
	WriteParameter("#P(1)", child, send2);
	WriteParameter("#m(1)", child, send2);
}
void CameraThread::SetPositiveLevel(const HWND &child, const HWND &send2)
{
	WriteParameter("#P(1)", child, send2);
	WriteParameter("#m(0)", child, send2);
}
void CameraThread::SetNegativeEdge(const HWND &child, const HWND &send2)
{
	WriteParameter("#P(0)", child, send2);
	WriteParameter("#m(1)", child, send2);
}
void CameraThread::SetNegativeLevel(const HWND &child, const HWND &send2)
{
	WriteParameter("#P(0)", child, send2);
	WriteParameter("#m(0)", child, send2);
}
void CameraThread::SetFrameFormat(size_t img_width, size_t img_height, const HWND &child, const HWND &send2)
{
	std::string text_tmp = std::string("#R(") + std::to_string(img_width) + std::string(",") + std::to_string(img_height) + std::string(")\r");
	WriteParameter(QString::fromStdString(text_tmp), child, send2);
}
void CameraThread::SetExposure(size_t exposure, const HWND &child, const HWND &send2)
{
	std::string text_tmp = std::string("#e(") + std::to_string(exposure) + std::string(")\r");
	WriteParameter(QString::fromStdString(text_tmp), child, send2);
}
void CameraThread::SetFrameRate(size_t frame_rate, const HWND &child, const HWND &send2)
{
	std::string text_tmp = std::string("#r(") + std::to_string(frame_rate) + std::string(")\r");
	WriteParameter(QString::fromStdString(text_tmp), child, send2);
}

void CameraThread::SetCameraParameter(const CameraParameters& cameraParams)
{
	mCameraParams = cameraParams;
	PvGenParameterArray *aDeviceParams = mpDevice->GetCommunicationParameters();
	aDeviceParams->SetBooleanValue("LinkRecoveryEnabled", true);
	PvGenParameterArray *bDeviceParams = mpDevice->GetParameters();
	bDeviceParams->SetEnumValue("TestPattern", "Off");
	bDeviceParams->SetEnumValue("SensorDigitizationTaps", 1);
	bDeviceParams->SetIntegerValue("Width", std::atoi(mCameraParams.imageWidth.c_str()));
	bDeviceParams->SetIntegerValue("Height", std::atoi(mCameraParams.imageHeight.c_str()));

	PvDeviceAdapter *lDeviceAdapter = new PvDeviceAdapter(mpDevice);
	PvSerialTerminalWnd mSerialTerminalWnd;
	mSerialTerminalWnd.SetDevice(lDeviceAdapter);
	mSerialTerminalWnd.ShowModeless();

	HWND fatherwnd = FindWindow(nullptr, _T("Device Serial Communication"));
	HWND child = FindWindowEx(fatherwnd, nullptr, _T("Edit"), nullptr);
	HWND send1 = FindWindowEx(fatherwnd, nullptr, _T("Button"), _T("Send"));
	HWND send2 = FindWindowEx(fatherwnd, send1, _T("Button"), _T("Send"));

	//FrameFormat
	//SetFrameFormat(std::atoi(mCameraParams.imageWidth.c_str()), std::atoi(mCameraParams.imageHeight.c_str()), child, send2);
	//Exposure
	SetExposure(std::atoi(mCameraParams.exposureTime.c_str()), child, send2);
	//FrameRate
	SetFrameRate(std::atoi(mCameraParams.frameFrequency.c_str()), child, send2);

	//Synchro
	if (mCameraParams.synchroMode == std::string("0"))
	{
		SetInternSynchro(child, send2);
	}
	else if (mCameraParams.synchroMode == std::string("1"))
	{
		SetExternSynchro(child, send2);
	}
	//Polarity
	if (mCameraParams.polarityMode == std::string("00"))
	{
		SetNegativeLevel(child, send2);
	}
	else if (mCameraParams.polarityMode == std::string("01"))
	{
		SetNegativeEdge(child, send2);
	}
	else if (mCameraParams.polarityMode == std::string("10"))
	{
		SetPositiveLevel(child, send2);
	}
	else if (mCameraParams.polarityMode == std::string("11"))
	{
		SetPositiveEdge(child, send2);
	}

	mSerialTerminalWnd.Close();
	delete lDeviceAdapter;
}