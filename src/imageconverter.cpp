#include "imageconverter.h"

namespace ImageConverter
{
	QImage cvMat2QImage(const cv::Mat& mat)
	{
		if (mat.type() == CV_8UC1)
		{
			QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
			image.setColorCount(256);
			for (int i = 0; i < 256; i++)
			{
				image.setColor(i, qRgb(i, i, i));
			}
			uchar *pSrc = mat.data;
			for (int row = 0; row < mat.rows; row++)
			{
				uchar *pDest = image.scanLine(row);
				memcpy(pDest, pSrc, mat.cols);
				pSrc += mat.step;
			}
			return image;
		}
		else if (mat.type() == CV_8UC3)
		{
			const uchar *pSrc = (const uchar*)mat.data;
			QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
			return image.rgbSwapped();
		}
		else
		{
			std::cout << "Unsupported mat type" << std::endl;
			return QImage();
		}
	}

	cv::Mat convertTo3Channels(const cv::Mat& binImg)
	{
		cv::Mat three_channel = cv::Mat::zeros(binImg.rows, binImg.cols, CV_8UC3);
		std::vector<cv::Mat> channels;
		for (int i = 0; i < 3; i++)
		{
			channels.push_back(binImg);
		}
		merge(channels, three_channel);
		return three_channel;
	}

	QImage Rawdata2QImage(unsigned char* pData, size_t width, size_t height, QImage::Format format)
	{
		QImage qImg = QImage(pData, width, height, format);
		Gray8bitQImageCorlorTable cvt;
		qImg.setColorTable(cvt.GetGray8bitQImageCorlorTable());
		return qImg;
	}

	QVector<QRgb> Gray8bitQImageCorlorTable::vcolorTable;
	bool Gray8bitQImageCorlorTable::IsConstructed(false);

	const QVector<QRgb>& Gray8bitQImageCorlorTable::GetGray8bitQImageCorlorTable()
	{
		if (!Gray8bitQImageCorlorTable::IsConstructed)
		{
			for (int i = 0; i < 256; ++i)
			{
				vcolorTable.append(qRgb(i, i, i));
			}
			IsConstructed = true;
		}
		return vcolorTable;
	}
}