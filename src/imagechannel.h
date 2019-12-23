#ifndef ITEMREPOSITORY_H
#define ITEMREPOSITORY_H

#include <QImage>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>
#include <iostream>

enum ConsumerType { DISPLAY, SAVE, PROCESS };

class ImageChannel
{
public:
	ImageChannel(size_t bufferSize) : mBufferSize(bufferSize), mDisplayPos(0), mSavePos(0), mProcessPos(0), mWritePos(0)
	{
		mItemBuffer.resize(mBufferSize);
	}
	~ImageChannel() {}

	int PushImage(QImage& imageData)
	{
		std::unique_lock<std::mutex> lock(mMutex);
		//while (((ir->write_position + 1) % kItemRepositorySize)
		//	== ir->read_position) { // item buffer is full, just wait here.
		//	std::cout << "Producer is waiting for an empty slot...\n";
		//	(ir->repo_not_full).wait(lock); // 生产者等待"产品库缓冲区不为满"这一条件发生.
		//}
		std::cout << "write pos: " << mWritePos << std::endl;
		mItemBuffer[mWritePos++] = imageData;

		if (mWritePos == mBufferSize)
			mWritePos = 0;

		repo_not_empty.notify_all();
		lock.unlock();
		return 0;
	}

	int ConsumeImage(QImage& imageData, ConsumerType type)
	{
		size_t* pPosition;
		switch (type)
		{
		case DISPLAY:
			pPosition = &mDisplayPos;
			break;
		case SAVE:
			pPosition = &mSavePos;
			break;
		case PROCESS:
			pPosition = &mProcessPos;
			break;
		default:
			std::cout << "Unknown consumer type" << std::endl;
			return -1;
		}

		std::unique_lock<std::mutex> lock(mMutex);
		// item buffer is empty, just wait here.
		while (mWritePos == *pPosition) {
			std::cout << "Consumer is waiting for items...\n";
			repo_not_empty.wait(lock);
		}

		std::cout << "consume pos: " << *pPosition << std::endl;
		imageData = mItemBuffer[(*pPosition)++];

		if (*pPosition >= mBufferSize)
			*pPosition = 0;

		//(ir->repo_not_full).notify_all();
		lock.unlock();

		return 0;
	}

private:
	std::vector<QImage> mItemBuffer;
	size_t mBufferSize;
	size_t mDisplayPos;
	size_t mSavePos;
	size_t mProcessPos;
	size_t mWritePos;
	std::mutex mMutex;
	std::condition_variable repo_not_full;
	std::condition_variable repo_not_empty;
};

typedef std::shared_ptr<ImageChannel> ImageChannelPtr;

#endif