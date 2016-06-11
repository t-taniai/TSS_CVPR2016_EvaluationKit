#pragma once
#include <opencv2/opencv.hpp>
#include <exception>
#include <stdlib.h>

namespace FlowIO
{
	struct CShape
	{
		int width, height;      // width and height in pixels
		int nBands;             // number of bands/channels

		// Constructors and helper functions 
		CShape(void) : width(0), height(0), nBands(0) {}
		CShape(int w, int h, int nb) : width(w), height(h), nBands(nb) {}
		bool InBounds(int x, int y);            // is given pixel address valid?
		bool InBounds(int x, int y, int band);  // is given pixel address valid?
		bool operator==(const CShape& ref);     // are two shapes the same?
		bool SameIgnoringNBands(const CShape& ref); // " ignoring the number of bands?
		bool operator!=(const CShape& ref);     // are two shapes not the same?
	};

	struct CError : public std::exception
	{
		CError(const char* msg)                 { strcpy(message, msg); }
		CError(const char* fmt, int d)          { sprintf(message, fmt, d); }
		CError(const char* fmt, float f)        { sprintf(message, fmt, f); }
		CError(const char* fmt, const char *s)  { sprintf(message, fmt, s); }
		CError(const char* fmt, const char *s,
			int d)                          {
			sprintf(message, fmt, s, d);
		}
		char message[1024];         // longest allowable message
	};

	// the "official" threshold - if the absolute value of either 
	// flow component is greater, it's considered unknown
	const int UNKNOWN_FLOW_THRESH = 1e9;

	// value to use to represent unknown flow
	const int UNKNOWN_FLOW = 1e10;

	// return whether flow vector is unknown
	bool unknown_flow(float u, float v);
	bool unknown_flow(float *f);

	cv::Mat unknown_flow_mask(cv::Mat flow);

	// read a flow file into 2-band image
	void ReadFlowFile(cv::Mat& img, const char* filename);

	// write a 2-band image into flow file 
	void WriteFlowFile(cv::Mat img, const char* filename);

	float ComputeMaxMotion(cv::Mat motim, cv::Mat& knownMask = cv::Mat(), cv::Mat& rad = cv::Mat());
	cv::Mat MotionToColor(cv::Mat motim, float maxmotion = -1, cv::Scalar bgColor = cv::Scalar());

	void computeColor(float fx, float fy, uchar *pix);
}
