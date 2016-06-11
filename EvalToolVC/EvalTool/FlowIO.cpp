// flow_io.cpp
//
// read and write our simple .flo flow file format

// ".flo" file format used for optical flow evaluation
//
// Stores 2-band float image for horizontal (u) and vertical (v) flow components.
// Floats are stored in little-endian order.
// A flow value is considered "unknown" if either |u| or |v| is greater than 1e9.
//
//  bytes  contents
//
//  0-3     tag: "PIEH" in ASCII, which in little endian happens to be the float 202021.25
//          (just a sanity check that floats are represented correctly)
//  4-7     width as an integer
//  8-11    height as an integer
//  12-end  data (width*height*2*4 bytes total)
//          the float values for u and v, interleaved, in row order, i.e.,
//          u[row0,col0], v[row0,col0], u[row0,col1], v[row0,col1], ...
//


// first four bytes, should be the same in little endian
#define TAG_FLOAT 202021.25  // check for this when READING the file
#define TAG_STRING "PIEH"    // use this when WRITING the file

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "flowIO.h"

using namespace FlowIO;

int ncols = 0;
const int MAXCOLS = 60;
int colorwheel[MAXCOLS][3];



void setcols(int r, int g, int b, int k)
{
	colorwheel[k][0] = b;
	colorwheel[k][1] = g;
	colorwheel[k][2] = r;
}

void makecolorwheel()
{
	// relative lengths of color transitions:
	// these are chosen based on perceptual similarity
	// (e.g. one can distinguish more shades between red and yellow 
	//  than between yellow and green)
	int RY = 15;
	int YG = 6;
	int GC = 4;
	int CB = 11;
	int BM = 13;
	int MR = 6;
	ncols = RY + YG + GC + CB + BM + MR;
	//printf("ncols = %d\n", ncols);
	if (ncols > MAXCOLS)
		exit(1);
	int i;
	int k = 0;
	for (i = 0; i < RY; i++) setcols(255, 255 * i / RY, 0, k++);
	for (i = 0; i < YG; i++) setcols(255 - 255 * i / YG, 255, 0, k++);
	for (i = 0; i < GC; i++) setcols(0, 255, 255 * i / GC, k++);
	for (i = 0; i < CB; i++) setcols(0, 255 - 255 * i / CB, 255, k++);
	for (i = 0; i < BM; i++) setcols(255 * i / BM, 0, 255, k++);
	for (i = 0; i < MR; i++) setcols(255, 0, 255 - 255 * i / MR, k++);
}

void FlowIO::computeColor(float fx, float fy, uchar *pix)
{
	if (ncols == 0)
		makecolorwheel();

	float rad = sqrt(fx * fx + fy * fy);
	float a = atan2(-fy, -fx) / M_PI;
	float fk = (a + 1.0) / 2.0 * (ncols - 1);
	int k0 = (int)fk;
	int k1 = (k0 + 1) % ncols;
	float f = fk - k0;
	//f = 0; // uncomment to see original color wheel
	for (int b = 0; b < 3; b++) {
		float col0 = colorwheel[k0][b] / 255.0;
		float col1 = colorwheel[k1][b] / 255.0;
		float col = (1 - f) * col0 + f * col1;
		if (rad <= 1)
			col = 1 - rad * (1 - col); // increase saturation with radius
		else
			col *= .75; // out of range
		pix[b] = (int)(255.0 * col);
	}
}


// return whether flow vector is unknown
bool FlowIO::unknown_flow(float u, float v) {
	return (fabs(u) >  FlowIO::UNKNOWN_FLOW_THRESH)
	|| (fabs(v) >  FlowIO::UNKNOWN_FLOW_THRESH)
	|| isnan(u) || isnan(v);
}

bool FlowIO::unknown_flow(float *f) {
	return FlowIO::unknown_flow(f[0], f[1]);
}


cv::Mat FlowIO::unknown_flow_mask(cv::Mat flow)
{
	cv::Mat mask = cv::Mat::zeros(flow.size(), CV_8UC1);
	std::vector<cv::Mat> ch;
	split(flow, ch);

	for (cv::Mat m : ch)
	{
		m = cv::abs(m);
		mask = mask | (m > FlowIO::UNKNOWN_FLOW_THRESH);
		mask = mask | (m != m);
	}
	return mask;
}

// read a flow file into 2-band image
void FlowIO::ReadFlowFile(cv::Mat& img, const char* filename)
{
    if (filename == NULL)
	throw CError("ReadFlowFile: empty filename");

    const char *dot = strrchr(filename, '.');
    if (strcmp(dot, ".flo") != 0)
	throw CError("ReadFlowFile (%s): extension .flo expected", filename);

    FILE *stream = fopen(filename, "rb");
    if (stream == 0)
        throw CError("ReadFlowFile: could not open %s", filename);
    
    int width, height;
    float tag;

    if ((int)fread(&tag,    sizeof(float), 1, stream) != 1 ||
	(int)fread(&width,  sizeof(int),   1, stream) != 1 ||
	(int)fread(&height, sizeof(int),   1, stream) != 1)
	throw CError("ReadFlowFile: problem reading file %s", filename);

    if (tag != TAG_FLOAT) // simple test for correct endian-ness
	throw CError("ReadFlowFile(%s): wrong tag (possibly due to big-endian machine?)", filename);

    // another sanity check to see that integers were read correctly (99999 should do the trick...)
    if (width < 1 || width > 99999)
	throw CError("ReadFlowFile(%s): illegal width %d", filename, width);

    if (height < 1 || height > 99999)
	throw CError("ReadFlowFile(%s): illegal height %d", filename, height);

    int nBands = 2;
    CShape sh(width, height, nBands);
	img.create(height, width, CV_MAKETYPE(CV_32F, nBands));

    //printf("reading %d x %d x 2 = %d floats\n", width, height, width*height*2);
    int n = nBands * width;
    for (int y = 0; y < height; y++) {
	float* ptr = (float *)&img.at<cv::Vec2f>(y, 0);
	if ((int)fread(ptr, sizeof(float), n, stream) != n)
	    throw CError("ReadFlowFile(%s): file is too short", filename);
    }

    if (fgetc(stream) != EOF)
	throw CError("ReadFlowFile(%s): file is too long", filename);

    fclose(stream);
}

// write a 2-band image into flow file 
void FlowIO::WriteFlowFile(cv::Mat img, const char* filename)
{
    if (filename == NULL)
	throw CError("WriteFlowFile: empty filename");

    const char *dot = strrchr(filename, '.');
    if (dot == NULL)
	throw CError("WriteFlowFile: extension required in filename '%s'", filename);

    if (strcmp(dot, ".flo") != 0)
	throw CError("WriteFlowFile: filename '%s' should have extension '.flo'", filename);

	cv::Size sh = img.size();
    int width = sh.width, height = sh.height, nBands = img.channels();

    if (nBands != 2)
	throw CError("WriteFlowFile(%s): image must have 2 bands", filename);

    FILE *stream = fopen(filename, "wb");
    if (stream == 0)
        throw CError("WriteFlowFile: could not open %s", filename);

    // write the header
    fprintf(stream, TAG_STRING);
    if ((int)fwrite(&width,  sizeof(int),   1, stream) != 1 ||
	(int)fwrite(&height, sizeof(int),   1, stream) != 1)
	throw CError("WriteFlowFile(%s): problem writing header", filename);

    // write the rows
    int n = nBands * width;
    for (int y = 0; y < height; y++) {
		float* ptr = (float*)&img.at<cv::Vec2f>(y, 0);
	if ((int)fwrite(ptr, sizeof(float), n, stream) != n)
	    throw CError("WriteFlowFile(%s): problem writing data", filename); 
   }

    fclose(stream);
}


float FlowIO::ComputeMaxMotion(cv::Mat motim, cv::Mat& knownMask, cv::Mat& rad)
{
	cv::Size sh = motim.size();
	int width = sh.width, height = sh.height;
	int x, y;

	// determine motion range:
	double maxx = -999, maxy = -999;
	double minx = 999, miny = 999;
	double maxrad = -1;

	cv::Mat unknownMask = unknown_flow_mask(motim);
	knownMask = ~unknownMask;
	std::vector<cv::Mat> bands;
	cv::split(motim, bands);
	cv::minMaxIdx(bands[0], &minx, &maxx, NULL, NULL, knownMask);
	cv::minMaxIdx(bands[1], &miny, &maxy, NULL, NULL, knownMask);
	cv::sqrt(bands[0].mul(bands[0]) + bands[1].mul(bands[1]), rad);
	cv::minMaxIdx(rad, NULL, &maxrad, NULL, NULL, knownMask);

	//std::cout << "max motion: " << maxrad << std::endl;

	return maxrad;
}

cv::Mat FlowIO::MotionToColor(cv::Mat motim, float maxmotion, cv::Scalar bgColor)
{
	cv::Size sh = motim.size();
	int width = sh.width, height = sh.height;
	int x, y;
	// determine motion range:
	double maxx = -999, maxy = -999;
	double minx = 999, miny = 999;
	cv::Mat knownMask;
	double maxrad = ComputeMaxMotion(motim, knownMask);

	if (maxmotion > 0) // i.e., specified on commandline
		maxrad = maxmotion;

	if (maxrad == 0) // if flow == 0 everywhere
		maxrad = 1;

	//if (verbose)
	//	fprintf(stderr, "normalizing by %g\n", maxrad);

	cv::Mat colim = cv::Mat(sh, CV_8UC3, bgColor);
	std::vector<cv::Mat> bands;
	cv::split(motim, bands);
	bands[0] = bands[0] / maxrad;
	bands[1] = bands[1] / maxrad;
	//rad = rad / maxrad;
	for (int y = 0; y < colim.rows; y++)
	for (int x = 0; x < colim.cols; x++)
	{
		if (knownMask.at<uchar>(y, x))
			FlowIO::computeColor(bands[0].at<float>(y, x), bands[1].at<float>(y, x), &colim.at<cv::Vec3b>(y, x)[0]);
	}
	return colim;
}
