#pragma once

#include <vector>
#include <opencv2\opencv.hpp>
#include <fstream>

namespace CvUtils
{
	cv::Mat channelDot(const cv::Mat& m1, const cv::Mat& m2)
	{
		cv::Mat m1m2 = m1.mul(m2);
		m1m2 = m1m2.reshape(1, m1.rows*m1.cols);
		cv::Mat m1m2dot;
		cv::reduce(m1m2, m1m2dot, 1, CV_REDUCE_SUM);
		return m1m2dot.reshape(1, m1.rows);
	}
	cv::Mat channelSum(const cv::Mat& m1)
	{
		cv::Mat m = m1.reshape(1, m1.rows*m1.cols);
		cv::reduce(m, m, 1, CV_REDUCE_SUM);
		return m.reshape(1, m1.rows);
	}

	cv::Mat ComputeValidFlowMask(cv::Mat flow)
	{
		cv::Mat u, v;
		cv::extractChannel(flow, u, 0);
		cv::extractChannel(flow, v, 1);
		return (cv::abs(u) <= 1e9) & (cv::abs(v) <= 1e9);
	}
	cv::Mat computeFlowError(cv::Mat flow, cv::Mat flowGT)
	{
		cv::Mat m = flow - flowGT;
		cv::Mat validGT = ComputeValidFlowMask(flowGT);
		cv::Mat valid = ComputeValidFlowMask(flow);
		m = channelSum(m.mul(m));
		cv::sqrt(m, m);

		//m.setTo(cv::Scalar(0), ~validGT);
		cv::bitwise_xor(validGT, valid, valid);
		m.setTo(cv::Scalar(1000), valid);

		return m;
	}

	template <typename T>
	cv::Mat CreateMeshgrid(int width, int height, int u_st = 0, int v_st = 0)
	{
		cv::Mat grid = cv::Mat_<cv::Vec<T, 2>>(height, width);
		for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			grid.at<cv::Vec<T, 2>>(y, x) = cv::Vec<T, 2>(x + u_st, y + v_st);
		return grid;
	}
	void ResizeFlow(const cv::Mat fi1, cv::Mat& resized1, cv::Size oldSize1, cv::Size oldSize2, cv::Size newSize1, cv::Size newSize2)
	{
		cv::Mat valid1;
		ComputeValidFlowMask(fi1).convertTo(valid1, CV_32F, 1.0 / 255);
		cv::resize(valid1, valid1, newSize1, 0, 0, cv::INTER_LINEAR);

		cv::resize(fi1, resized1, newSize1, 0, 0, cv::INTER_LINEAR);

		cv::Mat newGrid = CreateMeshgrid<float>(newSize1.width, newSize1.height);
		cv::Mat oldGrid = newGrid.mul(cv::Scalar((double)oldSize1.width / newSize1.width, (double)oldSize1.height / newSize1.height));

		resized1 = resized1 + oldGrid;
		resized1 = resized1.mul(cv::Scalar((double)newSize2.width / oldSize2.width, (double)newSize2.height / oldSize2.height)) - newGrid;

		// set the mixture flows at known/unknown boundary pixels to unknown
		resized1.setTo(cv::Scalar(1e10), valid1 != 1.0);
	}
	void ResizeFlowPair(cv::Mat& flow1, cv::Mat& flow2, cv::Size& newSize1, cv::Size& newSize2)
	{
		const cv::Size oldSize1 = flow1.size();
		const cv::Size oldSize2 = flow2.size();

		if (oldSize1 != newSize1 || oldSize2 != newSize2)
		{
			ResizeFlow(flow1, flow1, oldSize1, oldSize2, newSize1, newSize2);
			ResizeFlow(flow2, flow2, oldSize2, oldSize1, newSize2, newSize1);
		}
	}

	cv::Mat warpImage(cv::Mat flowMap, cv::Mat image, cv::Scalar borderValue = cv::Scalar())
	{
		cv::Mat_<float> map_x, map_y;
		cv::extractChannel(flowMap, map_x, 0);
		cv::extractChannel(flowMap, map_y, 1);
		for (int y = 0; y < map_x.rows; y++)
		for (int x = 0; x < map_x.cols; x++){
			map_x.at<float>(y, x) += x;
			map_y.at<float>(y, x) += y;
		}

		cv::Mat img;
		cv::remap(image, img, map_x, map_y, cv::InterpolationFlags::INTER_LINEAR, cv::BORDER_CONSTANT, borderValue);
		return img;
	}

	template <typename T>
	T convertStringToValue(std::string str)
	{
		return (T)std::stod(str);
	}
	template <> float convertStringToValue(std::string str) { return std::stof(str); }
	template <> int convertStringToValue(std::string str) { return std::stoi(str); }
	template <> std::string convertStringToValue(std::string str) { return str; }



}