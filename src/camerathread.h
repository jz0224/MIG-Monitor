#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <string>
#include <Windows.h>
#include <PvDevice.h>
#include <PvDeviceFinderWnd.h>
#include <PvSerialTerminalWnd.h>
#include <PvStream.h>
#include <PvDeviceAdapter.h>
#include <QString>
#include <PvPipeline.h>
#include <PvBuffer.h>
#include <PvStream.h>
#include <PvGenCommand.h>
#include <memory>

#include "imagechannel.h"

struct CameraParameters
{
	std::string imageWidth;		//string
	std::string imageHeight;		//string
	std::string frameFrequency;		//string
	std::string exposureTime;		//string
	std::string synchroMode;		//"0" 为内部，"1"为外部
	std::string polarityMode;		//"00" 为Negative Level，"01"为Negative Edge,"10"为Positive Level,"11"为Positive Edge
};

class CameraThread : public QObject
{
	Q_OBJECT
public:
	CameraThread(ImageChannelPtr pImageChannel);
	~CameraThread();

	void StartCapture();
	void StopCapture();
	void CaptureImage();
	bool IsCapturing();
	
	int ConnectToDevice();
	void Disconnect();
	void SetCameraParameter(const CameraParameters& para);

private:
	ImageChannelPtr mpImageChannel;
	std::thread mCaptureThread;

	bool mIsCapturing;
	PvDevice* mpDevice = nullptr;
	PvStream* mpStream = nullptr;
	PvPipeline *mpPipeline = nullptr;
	PvGenCommand* mpStartCommand = nullptr;
	PvGenCommand* mpStopCommand = nullptr;

	CameraParameters mCameraParams;

	void WriteParameter(QString cmd, const HWND& child, const HWND& send2);
	//    CameraParameters GetCameraParameter(PvDevice *lDevice);
	void SetExternSynchro(const HWND &child, const HWND &send2);
	void SetInternSynchro(const HWND &child, const HWND &send2);
	void SetPositiveEdge(const HWND &child, const HWND &send2);
	void SetPositiveLevel(const HWND &child, const HWND &send2);
	void SetNegativeEdge(const HWND &child, const HWND &send2);
	void SetNegativeLevel(const HWND &child, const HWND &send2);
	void SetFrameFormat(size_t img_width, size_t img_height, const HWND &child, const HWND &send2);
	void SetExposure(size_t exposure, const HWND &child, const HWND &send2);
	void SetFrameRate(size_t frame_rate, const HWND &child, const HWND &send2);

signals:
	
	void connectLost();
};

typedef std::shared_ptr<CameraThread> CameraThreadPtr;

#endif //CAPTURETHREAD_H