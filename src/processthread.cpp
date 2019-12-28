#include "processthread.h"
#include "imageconverter.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <time.h>
#include <windows.h>
#include <QPixmap>
#include <QMessageBox>
#include <PvPipeline.h>
#include <PvBuffer.h>
#include <PvDevice.h>
#include <PvStream.h>
#include <condition_variable>
#include <mutex>

using namespace cv;
using namespace std;

ProcessThread::ProcessThread(ImageChannelPtr imagechannel):
	mpImageChannel(imagechannel)
{
	mIsPlaying = false;
    mIsRecording = false;
	mIsProcessing = false;

	mSaveFreq = 1;
}

ProcessThread::~ProcessThread()
{

}

void ProcessThread::SetSaveFreq(size_t saveFreq)
{
	mSaveFreq = saveFreq;
}

void ProcessThread::SetSavePath(std::string& savePath)
{
	mSavePath = savePath;
}

void ProcessThread::SetProcessParam(ImageProcessParameters& ipParameters)
{
	// TODO：检查合法性
	mIPParameters = ipParameters;
}

void ProcessThread::StartDisplay()
{
	mIsPlaying = true;
	std::thread displayThread = std::thread(&ProcessThread::DisplayImage, this);
	displayThread.detach();
}

void ProcessThread::StopDisplay()
{
	mIsPlaying = false;
}

void ProcessThread::StartRecord()
{
	mIsRecording = true;
	std::thread processThread =  std::thread(&ProcessThread::SaveImage, this);
	processThread.detach();
}

void ProcessThread::StopRecord()
{
	mIsRecording = false;
}

void ProcessThread::StartProcess()
{
	mIsProcessing = true;
	std::thread processThread = std::thread(&ProcessThread::ProcessImage, this);
	processThread.detach();
}

void ProcessThread::StopProcess()
{
	mIsProcessing = false;
}

void ProcessThread::StopAllTasks()
{
	StopDisplay();
	StopRecord();
	StopProcess();
}

void ProcessThread::DisplayImage()
{
	while (mIsPlaying)
	{
		QImage qImg;
		int rtn = mpImageChannel->ConsumeImage(qImg, ConsumerType::DISPLAY);
		//std::cout << "display one image" << std::endl;
		emit sendImg(qImg);
	}
}

void ProcessThread::SaveImage()
{
	mSaveNum = 1;
	while (mIsRecording)
	{
		QImage qImg;
		int rtn = mpImageChannel->ConsumeImage(qImg, ConsumerType::SAVE);
		std::string str_tmp = mSavePath + "\\" + std::to_string(mSaveNum++) + ".TIFF";
		qImg.save(str_tmp.c_str());
	}
}

void ProcessThread::ProcessImage()
{
	while (mIsProcessing)
	{
		QImage qImg;
		int rtn = mpImageChannel->ConsumeImage(qImg, ConsumerType::PROCESS);
		IPThreshold(qImg);
	}
}

int ProcessThread::IPThreshold(QImage& origin)
{
	cv::Mat src = cv::Mat(origin.height(), origin.width(), CV_8UC1, (void*)origin.bits(), origin.bytesPerLine()).clone();
	if (!src.data) return -1;
	size_t real_x = min(mIPParameters.roi_x, max(0, origin.width() - mIPParameters.roi_w));
	size_t real_y = min(mIPParameters.roi_y, max(0, origin.height() - mIPParameters.roi_h));
	cv::Mat src_roi = src(cv::Rect(real_x, real_y, mIPParameters.roi_w, mIPParameters.roi_h));

	cv::Mat Thre;
	cv::threshold(src_roi, Thre, mIPParameters.ip_threshold, 255, CV_THRESH_BINARY);
	cv::Mat rgb = ImageConverter::convertTo3Channels(Thre);
	cv::Rect box = boundingRect(Thre);
	cv::rectangle(rgb, cv::Point(box.x, box.y), cv::Point(box.x + box.width, box.y + box.height), cv::Scalar(0, 255, 0), 2, 8);
	QImage qroi = ImageConverter::cvMat2QImage(rgb);
	int width = box.width;
	int length = box.height;

	emit sendROI(qroi);
	emit sendWidth(width);
	emit sendLength(length);

	return 0;
}
