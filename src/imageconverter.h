#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include <QImage>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <vector>

namespace ImageConverter
{
	QImage cvMat2QImage(const cv::Mat& mat);

	cv::Mat convertTo3Channels(const cv::Mat& binImg);

	QImage Rawdata2QImage(unsigned char* pData, size_t width, size_t height, QImage::Format format);

	class Gray8bitQImageCorlorTable {
	public:
		static QVector<QRgb> vcolorTable;
		static bool IsConstructed;
		const QVector<QRgb>& GetGray8bitQImageCorlorTable();
	};
}

#endif