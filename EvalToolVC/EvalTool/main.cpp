#include <opencv2/opencv.hpp>

#include "FlowIO.h"
#include "WinUtils.h"
#include "ArgsParser.h"
#include "CvUtils.h"
#include <direct.h>

using namespace std;
using namespace cv;

cv::Scalar BGCOLOR = cv::Scalar(255, 255, 0);
cv::Scalar FLBGCOLOR = cv::Scalar(128, 128, 128);
bool autoFlip = false;

void load_data(string dir, cv::Mat& flow1, cv::Mat& flow2, cv::Mat& mask1, cv::Mat& mask2, string& image1 = string(), string& image2 = string())
{
	try {
		mask1 = imread(dir + "\\mask1.png", cv::IMREAD_GRAYSCALE);
		mask2 = imread(dir + "\\mask2.png", cv::IMREAD_GRAYSCALE);
	}
	catch (std::exception){
		mask1 = cv::Mat();
		mask2 = cv::Mat();
	}
	try {
		FlowIO::ReadFlowFile(flow1, (dir + "\\flow1.flo").c_str());
		FlowIO::ReadFlowFile(flow2, (dir + "\\flow2.flo").c_str());
	}
	catch (std::exception){
		flow1 = cv::Mat();
		flow2 = cv::Mat();
	}

	FILE *fp = fopen((dir + "\\pair.txt").c_str(), "r");
	if (fp != nullptr)
	{
		char buff[2][512];
		fscanf(fp, "%[^,],%[^,\n]\n", buff[0], buff[1]);
		fscanf(fp, "%[^,],%[^,\n]\n", buff[0], buff[1]);
		fclose(fp);
		image1 = buff[0];
		image2 = buff[1];
	}
}

cv::Mat_<double> compute_score(cv::Mat maskGT1, cv::Mat flowGT1, cv::Mat mask1, cv::Mat flow1, cv::Mat thresholds)
{
	cv::Mat_<double> s(thresholds.rows + 1, thresholds.cols);
	s = 0;

	if (!mask1.empty())
	{
		s.at<double>(0) = (double)cv::countNonZero(maskGT1 & mask1) / cv::countNonZero(maskGT1 | mask1);
	}

	if (!flow1.empty())
	{
		cv::Mat error = CvUtils::computeFlowError(flow1, flowGT1);
		cv::Mat validGT = CvUtils::ComputeValidFlowMask(flowGT1);

		double validSize = cv::countNonZero(validGT);

		for (int i = 0; i < thresholds.rows; i++)
			s.at<double>(i + 1) = 1.0 - (double)cv::countNonZero((error > thresholds.at<double>(i)) & validGT) / validSize;
	}

	return s;
}


void output_visualization(cv::Mat mask1, cv::Mat flow1, cv::Mat image1, cv::Mat image2, std::string dir, std::string suffix, float maxmotion = -1)
{
	if (!mask1.empty())
	{
		cv::Mat foreground = image1.clone();
		foreground.setTo(BGCOLOR, ~mask1);
		cv::imwrite(dir + "\\foreground" + suffix + ".png", foreground);
	}
	else
		mask1 = cv::Mat(flow1.size(), CV_8U, cv::Scalar(255));

	if (!flow1.empty())
	{
		cv::Mat warped = CvUtils::warpImage(flow1, image2, BGCOLOR);
		warped.setTo(BGCOLOR, ~mask1);
		cv::imwrite(dir + "\\warped" + suffix + ".png", warped);

		cv::Mat flow = FlowIO::MotionToColor(flow1, maxmotion, FLBGCOLOR);
		flow.setTo(FLBGCOLOR, ~mask1);
		cv::imwrite(dir + "\\flow" + suffix + ".png", flow);
	}
}

void output_visualization(string srcDir, string desDir, string datasetDir)
{
	cv::Mat mask1, mask2, flow1, flow2;

	load_data(srcDir, flow1, flow2, mask1, mask2);
	cv::Mat image1 = cv::imread(datasetDir + "\\image1.png");
	cv::Mat image2 = cv::imread(datasetDir + "\\image2.png");

	if (flow1.empty() && flow2.empty() && mask1.empty() && mask2.empty())
		return;

	if (!mask1.empty() && mask1.size() != image1.size())
		cv::resize(image1, image1, mask1.size());
	else if (!flow1.empty() && flow1.size() != image1.size())
		cv::resize(image1, image1, flow1.size());

	if (!mask2.empty() && mask2.size() != image2.size())
		cv::resize(image2, image2, mask2.size());
	else if (!flow2.empty() && flow2.size() != image2.size())
		cv::resize(image2, image2, flow2.size());

	// When visualizing flow map, we need flowGT to get its max motion value
	float maxmotion = -1;
	{
		cv::Mat maskGT1, maskGT2, flowGT1, flowGT2;
		string name1, name2;
		load_data(datasetDir, flowGT1, flowGT2, maskGT1, maskGT2, name1, name2);

		if (!flowGT1.empty() && !flowGT2.empty())
		{
			if (flowGT1.size() != image1.size() || flowGT2.size() != image2.size())
				CvUtils::ResizeFlowPair(flowGT1, flowGT2, image1.size(), image2.size());

			float maxmotion1 = FlowIO::ComputeMaxMotion(flowGT1);
			float maxmotion2 = FlowIO::ComputeMaxMotion(flowGT2);
			maxmotion = std::max({ maxmotion1, maxmotion2 });
		}

		if (autoFlip && !maskGT1.empty() && !maskGT2.empty() && !mask1.empty() && !mask2.empty())
		{
			if (maskGT1.size() != mask1.size()){
				cv::resize(maskGT1, maskGT1, mask1.size(), 0);
				maskGT1 = maskGT1 > 128;
			}
			if (maskGT2.size() != mask2.size()) {
				cv::resize(maskGT2, maskGT2, mask2.size(), 0);
				maskGT2 = maskGT2 > 128;
			}

			double score1_1 = compute_score(maskGT1, cv::Mat(), mask1, cv::Mat(), cv::Mat_<double>(0, 1))(0);
			double score2_1 = compute_score(maskGT2, cv::Mat(), mask2, cv::Mat(), cv::Mat_<double>(0, 1))(0);
			double score1_0 = compute_score(maskGT1, cv::Mat(), ~mask1, cv::Mat(), cv::Mat_<double>(0, 1))(0);
			double score2_0 = compute_score(maskGT2, cv::Mat(), ~mask2, cv::Mat(), cv::Mat_<double>(0, 1))(0);
			if (score1_1 + score2_1 < score1_0 + score2_0){
				mask1 = ~mask1;
				mask2 = ~mask2;
			}
		}
	}

	_mkdir((desDir).c_str());
	output_visualization(mask1, flow1, image1, image2, desDir, "1", maxmotion);
	output_visualization(mask2, flow2, image2, image1, desDir, "2", maxmotion);
}

void run_visualization(string resultsDir, string datasetDir, string subOutputDir = "")
{
	printf("Creating visualization.......\n");

	auto dirs = WinUtil::GetDirectries(resultsDir, "\\*");

	for (int i = 0; i < dirs.size(); i++)
	{
		string _srcDir = resultsDir + "\\" + dirs[i];
		string _desDir = resultsDir + "\\" + dirs[i] + "\\" + subOutputDir;
		string _dataDir = datasetDir + "\\" + dirs[i];
		output_visualization(_srcDir, _desDir, _dataDir);
	}
}

void computeMaskFromFlow(cv::Mat flow1, cv::Mat flow2, cv::Mat& mask1, cv::Mat& mask2, double thres)
{
	if (flow1.empty() || flow2.empty()){
		mask1 = cv::Mat();
		mask2 = cv::Mat();
		return;
	}

	cv::Mat warpedFlow1 = CvUtils::warpImage(flow1, flow2, cv::Scalar::all(1e10));
	cv::Mat warpedFlow2 = CvUtils::warpImage(flow2, flow1, cv::Scalar::all(1e10));

	mask1 = CvUtils::computeFlowError(flow1, -warpedFlow1) < thres;
	mask2 = CvUtils::computeFlowError(flow2, -warpedFlow2) < thres;
}

void run_evaluation(string resultDir, string datasetDir)
{
	printf("Evaluating results.......\n");

	auto dirs = WinUtil::GetDirectries(resultDir, "\\*");
	FILE *scoreTable = fopen((resultDir + "\\scores.csv").c_str(), "w");
	const int THRESHOLD = 50;

	if (scoreTable == nullptr)
	{
		printf("Failed to open the output file: %s\n", (resultDir + "\\scores.csv").c_str());
		printf("Evaluation terminated.\n");
		return;
	}

	fprintf(scoreTable, "%s,%s,%s,%s,%s", "Row", "Src", "Ref", "SegIUR", "Flip");

	cv::Mat_<double> thresholds(THRESHOLD, 1);
	for (int i = 0; i < THRESHOLD; i++) {
		fprintf(scoreTable, ",T%d", i + 1);
		thresholds.at<double>(i) = i + 1;
	}
	fprintf(scoreTable, "\n");

	cv::Mat_<double> meanScore = cv::Mat_<double>::zeros(THRESHOLD + 1, 1);
	cv::Mat_<double> meanNoFlipScore = cv::Mat_<double>::zeros(THRESHOLD + 1, 1);
	cv::Mat_<double> score(meanScore.size());
	int count = 0;
	int NoFlipCount = 0;
	for (int i = 0; i < dirs.size(); i++)
	{
		string _srcDir = datasetDir + "\\" + dirs[i];
		string _desDir = resultDir + "\\" + dirs[i];

		cv::Mat maskGT1, maskGT2, flowGT1, flowGT2;
		cv::Mat mask1, mask2, flow1, flow2;
		string name1, name2;
		load_data(_srcDir, flowGT1, flowGT2, maskGT1, maskGT2, name1, name2);
		load_data(_desDir, flow1, flow2, mask1, mask2);

		if (flowGT1.empty() || flowGT2.empty() || maskGT1.empty() || maskGT2.empty())
			continue;

		int flip = 0;
		FILE *flipFp = fopen((_srcDir + "\\flip_gt.txt").c_str(), "r");
		if (flipFp != nullptr)
		{
			//char buff[2][512];
			fscanf(flipFp, "%d", &flip);
		}
		if (!mask1.empty() && mask1.size() != maskGT1.size())
		{
			cv::resize(mask1, mask1, maskGT1.size(), 0);
			mask1 = mask1 > 128;
		}
		if (!mask2.empty() && mask2.size() != maskGT2.size())
		{
			cv::resize(mask2, mask2, maskGT2.size());
			mask2 = mask2 > 128;
		}

		if (!flow1.empty() && !flow2.empty())
			CvUtils::ResizeFlowPair(flow1, flow2, flowGT1.size(), flowGT2.size());

		if (autoFlip && !maskGT1.empty() && !maskGT2.empty() && !mask1.empty() && !mask2.empty())
		{
			double score1_1 = compute_score(maskGT1, cv::Mat(), mask1, cv::Mat(), cv::Mat_<double>(0, 1))(0);
			double score2_1 = compute_score(maskGT2, cv::Mat(), mask2, cv::Mat(), cv::Mat_<double>(0, 1))(0);
			double score1_0 = compute_score(maskGT1, cv::Mat(), ~mask1, cv::Mat(), cv::Mat_<double>(0, 1))(0);
			double score2_0 = compute_score(maskGT2, cv::Mat(), ~mask2, cv::Mat(), cv::Mat_<double>(0, 1))(0);
			if (score1_1 + score2_1 < score1_0 + score2_0){
				mask1 = ~mask1;
				mask2 = ~mask2;
			}
		}

		if (mask1.empty() || mask2.empty())
			computeMaskFromFlow(flow1, flow2, mask1, mask2, 20);

		score = compute_score(maskGT1, flowGT1, mask1, flow1, thresholds / 100.0 * (double)std::max(flowGT2.rows, flowGT2.cols));
		fprintf(scoreTable, "%s_1to2,%s,%s,%lf,%d", dirs[i].c_str(), name1.c_str(), name2.c_str(), score.at<double>(0), flip);
		for (int j = 0; j < THRESHOLD; j++) { fprintf(scoreTable, ",%lf", score.at<double>(j + 1)); } fprintf(scoreTable, "\n");
		meanScore += score;
		if (flip == 0) meanNoFlipScore += score;

		score = compute_score(maskGT2, flowGT2, mask2, flow2, thresholds / 100.0 * (double)std::max(flowGT1.rows, flowGT1.cols));
		fprintf(scoreTable, "%s_1to2,%s,%s,%lf,%d", dirs[i].c_str(), name2.c_str(), name1.c_str(), score.at<double>(0), flip);
		for (int j = 0; j < THRESHOLD; j++) { fprintf(scoreTable, ",%lf", score.at<double>(j + 1)); } fprintf(scoreTable, "\n");
		meanScore += score;
		if (flip == 0) meanNoFlipScore += score;
		if (flip == 0) NoFlipCount++;

		count++;
	}
	meanScore = meanScore / (count * 2.0);
	meanNoFlipScore = meanNoFlipScore / (NoFlipCount * 2.0);
	score = meanScore;
	fprintf(scoreTable, "%s,%s,%s,%lf,%d", "Average", "-", "-", score.at<double>(0), 1);
	for (int i = 0; i < THRESHOLD; i++) { fprintf(scoreTable, ",%lf", score.at<double>(i + 1)); } fprintf(scoreTable, "\n");
	fprintf(scoreTable, "%s,%s,%s,%lf,%d", "w/o flip", "-", "-", meanNoFlipScore.at<double>(0), 0);
	for (int i = 0; i < THRESHOLD; i++) { fprintf(scoreTable, ",%lf", meanNoFlipScore.at<double>(i + 1)); } fprintf(scoreTable, "\n");
	fclose(scoreTable);

	printf("------------- Score Summary ----------------------\n");
	printf("%8s %8s %8s %8s %8s %8s\n", "IUR", "FA1", "FA2", "FA3", "FA4", "FA5");
	printf("%8.3lf %8.3lf %8.3lf %8.3lf %8.3lf %8.3lf\n", score.at<double>(0), score.at<double>(1), score.at<double>(2), score.at<double>(3), score.at<double>(4), score.at<double>(5));

}

int main(int argn, char** args)
{
	ArgsParser argParser(argn, args);

	std::string resultsDir = "";
	std::string datasetDir = "";

	bool dir1 = argParser.TryGetArgment("resultsDir", resultsDir);
	bool dir2 = argParser.TryGetArgment("datasetDir", datasetDir);

	if (!dir1 || !dir2){
		std::cout << "Please specify -resultsDir and -datasetDir argments." << std::endl;
		return 1;
	}

	std::cout << "Root Directory of Results    : " << resultsDir << std::endl;
	std::cout << "Root Directory of Dataset    : " << datasetDir << std::endl;

	std::string mode = "evaluation";
	argParser.TryGetArgment("mode", mode);
	argParser.TryGetArgment("autoFlip", autoFlip); // Use only when cosegmentation methods are not aware which of 0/1 is the foreground label.


	if (mode == "evaluation")
	{
		printf("\n");
		run_evaluation(resultsDir, datasetDir);
	}
	else if (mode == "visualization")
	{
		std::string visSubDir = "";
		argParser.TryGetArgment("visSubDir", visSubDir);

		int bgColorCode;
		if( argParser.TryGetArgment("bgColor", bgColorCode))
			BGCOLOR = cv::Scalar(bgColorCode % 1000, (bgColorCode / 1000) % 1000, (bgColorCode / 1000000) % 1000);

		int flbgColorCode;
		if (argParser.TryGetArgment("flbgColor", flbgColorCode))
			FLBGCOLOR = cv::Scalar(flbgColorCode % 1000, (flbgColorCode / 1000) % 1000, (flbgColorCode / 1000000) % 1000);

		printf("Background Color of Image    : (R:%03d, G:%03d, B:%03d)\n", (int)BGCOLOR[2], (int)BGCOLOR[1], (int)BGCOLOR[0]);
		printf("Background Color of Flow Map : (R:%03d, G:%03d, B:%03d)\n", (int)FLBGCOLOR[2], (int)FLBGCOLOR[1], (int)FLBGCOLOR[0]);
		printf("Output Subdirectory Name     : %s\n", visSubDir.c_str());

		printf("\n");
		run_visualization(resultsDir, datasetDir, visSubDir);
	}

	return 0;
}
