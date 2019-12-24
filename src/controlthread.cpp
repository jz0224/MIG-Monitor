#include "controlthread.h"
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QTimer>
#include <vector>
#include <iostream>

const int sectionCount = 4;
const int channelCount = 2;
const int channelStart = 8;
ValueRange valueRange = V_0To5;
const int clockRatePerChan = 50000;
const int displayRate = 20;

ControlThread::ControlThread()
{
    mIsCollecting = false;
	mIsSaving = false;
}

ControlThread::~ControlThread()
{
	Sleep(1000);
	if (mpBufferedAiCtrl != nullptr)
    {
        mpBufferedAiCtrl->Dispose();
        mpBufferedAiCtrl = nullptr;
    }

    if (mpScaledData != nullptr)
    {
        delete[]mpScaledData;
        mpScaledData = nullptr;
    }
}

void ControlThread::DataHandler(void *sender, BfdAiEventArgs *args)
{
    ((BufferedAiCtrl*)sender)->GetData(args->Count, mpScaledData);
    int i = 0;
    int ratePerChannel = args->Count / channelCount;
    int dataPerRound = ratePerChannel / displayRate;
    int total = dataPerRound, inter = dataPerRound;
    while (i < args->Count) {
        std::vector<double> AI(channelCount, 0);
        for (; i < total; ++i) {
            AI[i % channelCount] += mpScaledData[i];
			if (mIsSaving == true)
			{
				mTextStream << mpScaledData[i] << ",";
				if (i % 2 == 1) mTextStream << '\n';
			}
        }
        mAnalogData.resize(channelCount + 1);
        for (int j = 0; j < channelCount; ++j) {
            mAnalogData[j] = AI[j] / dataPerRound*2;
        }
		double t = QString::number(mTime.elapsed() / 1000.0, 10, 3).toDouble();
		mAnalogData[channelCount] = t;
        emit SendAnalogData(mAnalogData);
        total += inter;
    }
}

QString ControlThread::GetDeviceName()
{
	return mDeviceName;
}

int ControlThread::Initialize()
{
    dataReadyHandler.owner = this;
    overrunHandler.owner = this;
    cacheOverflowHandler.owner = this;
    stoppedHander.owner = this;

    //QString deviceName = { "USB-4711A,BID#0" };
	mDeviceName = { "DemoDevice,BID#0" };
    std::wstring wDeviceName = mDeviceName.toStdWString();
    DeviceInformation deviceInfo(wDeviceName.c_str());

    //#define deviceDescription L"DemoDevice,BID#0"
    //DeviceInformation selected(deviceDescription);

    mpBufferedAiCtrl = AdxBufferedAiCtrlCreate();

    mpBufferedAiCtrl->addDataReadyListener(dataReadyHandler);
    mpBufferedAiCtrl->addOverrunListener(overrunHandler);
    mpBufferedAiCtrl->addCacheOverflowListener(cacheOverflowHandler);
    mpBufferedAiCtrl->addStoppedListener(stoppedHander);

	ErrorCode errorCode = mpBufferedAiCtrl->setSelectedDevice(deviceInfo);
	if (errorCode >= 0xE0000000 && errorCode != Success)
	{
		return -1;
	}

    int sectionLengthPerChan = clockRatePerChan / sectionCount;
    int rawDataBufferLength = sectionLengthPerChan * channelCount * sectionCount;
    mpScaledData = new double[rawDataBufferLength];

    mpBufferedAiCtrl->setStreaming(true);
    mpBufferedAiCtrl->getScanChannel()->setChannelCount(channelCount);
    mpBufferedAiCtrl->getScanChannel()->setChannelStart(channelStart);
    mpBufferedAiCtrl->getScanChannel()->setSamples(clockRatePerChan);
    mpBufferedAiCtrl->getScanChannel()->setIntervalCount(sectionLengthPerChan);
    mpBufferedAiCtrl->getConvertClock()->setRate(clockRatePerChan);

    for (int i = 0; i < mpBufferedAiCtrl->getChannels()->getCount(); i++)
    {
        mpBufferedAiCtrl->getChannels()->getItem(i).setValueRange(valueRange);
    }
    mpBufferedAiCtrl->Prepare();

	return 0;
}

void ControlThread::SetSavePath(std::string& savePath)
{
	mSavePath = savePath;
}

void ControlThread::StartCollect()
{
	mIsCollecting = true;
	mTime.start();
	mpBufferedAiCtrl->Start();
}

void ControlThread::StopCollect()
{
	mIsCollecting = false;
	mpBufferedAiCtrl->Stop();
}

bool ControlThread::IsCollecting()
{
	return mIsCollecting == true;
}

void ControlThread::StartSaving()
{
	mFile.setFileName(QString::fromStdString(mSavePath));
	mTextStream.setDevice(&mFile);
	mTextStream << "Current,Voltage\n";
	mFile.close();
	mFile.open(QIODevice::WriteOnly | QIODevice::Text);
}

void ControlThread::StopSaving()
{
	mFile.flush();
	Sleep(1000);
	mFile.close();

	emit SendFinishSignal();
}
