#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H

#include <QThread>
#include <QPixmap>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <memory>
#include <thread>

#include "imagechannel.h"

struct ImageProcessParameters
{
	unsigned short roi_x;
	unsigned short roi_y;
	unsigned short roi_w;
	unsigned short roi_h;
	unsigned short ip_threshold;
};

class ProcessThread : public QObject
{
    Q_OBJECT

public:
	ProcessThread(ImageChannelPtr imagechannel);
    ~ProcessThread();

	std::thread CreateDisplayThread();
	std::thread CreateSaveThread();
	std::thread CreateProcessThread();

	void DisplayImage();
	void SaveImage();
	void ProcessImage();

	void SetSaveFreq(size_t saveFreq);
	void SetSavePath(std::string& savePath);
	void SetProcessParam(ImageProcessParameters& ipParameters);
	void StartDisplay();
	void StopDisplay();
	void StartRecord();
	void StopRecord();
	void StartProcess();
	void StopProcess();

	void StopAllTasks();

	int IPThreshold(QImage& origin);

private:
	ImageChannelPtr mpImageChannel;

	std::thread mDisplayThread;
	std::thread mSaveThread;
	std::thread mProcessThread;

	ImageProcessParameters mIPParameters;

	bool mIsRecording;
	bool mIsProcessing;
	bool mIsPlaying;

	std::string mSavePath;
	size_t mSaveFreq;
	size_t mSaveNum;

signals:
    void sendImg(QImage);
	void sendWidth(double);
	void sendLength(double);
	void sendROI(QImage);

private slots:

};
#endif // PROCESSTHREAD_H
