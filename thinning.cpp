#include "thinning.h"
#include<iostream>
#include<opencv2/opencv.hpp>

namespace Myximgproc {

	// Applies a thinning iteration to a binary image
	static void thinningIteration(cv::Mat img, int iter, int thinningType) {
		cv::Mat marker = cv::Mat::zeros(img.size(), CV_8UC1);

		if (thinningType == THINNING_ZHANGSUEN) {
			for (int i = 1; i < img.rows - 1; i++) {
				for (int j = 1; j < img.cols - 1; j++) {
					uchar p2 = img.at<uchar>(i - 1, j);
					uchar p3 = img.at<uchar>(i - 1, j + 1);
					uchar p4 = img.at<uchar>(i, j + 1);
					uchar p5 = img.at<uchar>(i + 1, j + 1);
					uchar p6 = img.at<uchar>(i + 1, j);
					uchar p7 = img.at<uchar>(i + 1, j - 1);
					uchar p8 = img.at<uchar>(i, j - 1);
					uchar p9 = img.at<uchar>(i - 1, j - 1);

					int A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
						(p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
						(p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
						(p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
					int B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
					int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
					int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

					if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
						marker.at<uchar>(i, j) = 1;
				}
			}
		}
		if (thinningType == THINNING_GUOHALL) {
			for (int i = 1; i < img.rows - 1; i++) {
				for (int j = 1; j < img.cols - 1; j++) {
					uchar p2 = img.at<uchar>(i - 1, j);
					uchar p3 = img.at<uchar>(i - 1, j + 1);
					uchar p4 = img.at<uchar>(i, j + 1);
					uchar p5 = img.at<uchar>(i + 1, j + 1);
					uchar p6 = img.at<uchar>(i + 1, j);
					uchar p7 = img.at<uchar>(i + 1, j - 1);
					uchar p8 = img.at<uchar>(i, j - 1);
					uchar p9 = img.at<uchar>(i - 1, j - 1);

					int C = ((!p2) & (p3 | p4)) + ((!p4) & (p5 | p6)) +
						((!p6) & (p7 | p8)) + ((!p8) & (p9 | p2));
					int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
					int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
					int N = N1 < N2 ? N1 : N2;
					int m = iter == 0 ? ((p6 | p7 | (!p9)) & p8) : ((p2 | p3 | (!p5)) & p4);

					if ((C == 1) && ((N >= 2) && ((N <= 3)) & (m == 0)))
						marker.at<uchar>(i, j) = 1;
				}
			}
		}
		img &= ~marker;
		//cvNamedWindow("marker",2);imshow("marker",(marker*255));cvWaitKey(0);
		//cout<<marker*255<<endl;
	}

	// Apply the thinning procedure to a given image
	void thinning(cv::Mat input, cv::Mat& output, int thinningType) {
		cv::Mat processed = input.clone();
		// Enforce the range of the input image to be in between 0 - 255
		processed /= 255;

		cv::Mat prev = cv::Mat::zeros(processed.size(), CV_8UC1);
		cv::Mat diff;

		do {
			thinningIteration(processed, 0, thinningType);
			thinningIteration(processed, 1, thinningType);
			absdiff(processed, prev, diff);
			processed.copyTo(prev);
		} while (countNonZero(diff) > 0);

		processed *= 255;
		output = processed.clone();
		//cvNamedWindow("process",2);imshow("process",output);cvWaitKey(0);
	}

	void cutTwigs(cv::Mat& img) 
	{
		std::vector<cv::Point>pts;
		cv::findNonZero(img, pts);
		std::vector<cv::Point>leaves;
		cv::Point nbs[8] = { {0,-1},{-1,-1},{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1} };

		auto imgRect = cv::Rect{ 0,0,img.cols,img.rows };
		for (auto& pt : pts)
		{
			if (imgRect.contains(cv::Point(pt.x - 1, pt.y - 1)))
			{
				cv::Rect neighbourRect{ pt.x - 1,pt.y - 1,3,3 };
				if (cv::countNonZero(img(neighbourRect)) == 2
					and img.at<uchar>(pt) == 255)
				{
					leaves.emplace_back(pt);
				}
			}
		}


		//叶子往树枝方向剪切
		for (auto& leaf : leaves) {
			cv::Point curPt = leaf, nextPt = leaf;

			while (true) 
			{
				if ((int)nextPt.x < 0) break;
				img.at<uchar>(curPt) = 0;
				curPt = nextPt;

				int nbs_num{ 0 };
				/*for (int i = -1; i <= 1; i++)
				{
					for (int j = -1; j <= 1; j++) {
						if (i == 0 and j == 0)
							continue;

						cv::Point pt = curPt + cv::Point{i,j};

						if ((unsigned)(pt.x) >= img.cols or (unsigned)(pt.y) >= img.rows)
							continue;

						if (img.at<uchar>(pt) == 255)
						{
							nbs_num++;
							nextPt = pt;
						}
					}
				}*/

				for (size_t i = 0; i < 8; i++)
				{
					cv::Point pt = curPt + nbs[i];
					if (imgRect.contains(pt) and img.at<uchar>(pt) == 255)
					{
						nbs_num++;
						nextPt = pt;
					}
				}

				if (nbs_num != 1)
					break;
			}
		}
	}

}
