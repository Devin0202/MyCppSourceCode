/**
 * @file
 * @author	Devin dai
 * @version	0.0.1
 * @date	2020/05/29
 * @brief	Blurness detection based on opencv.
 * @details	"bdlapv" means blurness detection with
 * 			laplacian transform and standard variance.
 * 			We want to how blur the image(patch) is,
 * 			and give a score as result.
 * @section LICENSE
 * Copyright 2020 Hiscene
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * 		http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "bdlapv.h"

/**
 * @brief			Expanding rectangle with fixed center.
 * @param[in] rect 	Original rectangle @see cv::Rect.
 * @param[in] ratio Expanding ratio to each edge.
 * @return			Expanded rectangle.
 */
cv::Rect rectCenterScale(cv::Rect rect, float ratio) {
	if (0 > ratio) {
		return rect;
	} else {
		int deltaW = cvRound((ratio - 1) * rect.width);
		int deltaH = cvRound((ratio - 1) * rect.height);
		cv::Point pt;
		pt.x = cvRound(deltaW / 2.0);
		pt.y = cvRound(deltaW / 2.0);
		rect.width += deltaW;
		rect.height += deltaH;
		return (rect - pt);
	}
}

/**
 * @brief					Get the minimum rectangle containing landmarks.
 * @param[in] oriImage		Original image.
 * @param[in] faceRect		The rectangle with absolute ordinates to oriImage.
 * 							All landmarks in this rectangle.
 * @param[in] landmarks		Float array contains landmarks.
 * 							Relative ordinates to @see faceRect.
 * 							Format as [x, x, x, ..., y, y, y, ...].
 * @param[in] landmarksLen	Number of landmarks.
 * @return 					Minimum rectangle containing all landmarks,
 * 							with absolute ordinates to oriImage.
 */
cv::Rect getRoi(const cv::Mat &oriImage, const cv::Rect &faceRect,
				float* landmarks, int landmarksLen) {
	cv::Rect landMarkRect(faceRect.width, faceRect.height, 0, 0);
	for (int i = 0; i < landmarksLen; i++) {
		int xW = faceRect.x + int(landmarks[i] * faceRect.width);
		int yH = faceRect.y + int(landmarks[i + landmarksLen] * faceRect.height);
		landMarkRect.x = landMarkRect.x < xW ? landMarkRect.x : xW;
		landMarkRect.y = landMarkRect.y < yH ? landMarkRect.y : yH;
		landMarkRect.width = landMarkRect.width > xW ? landMarkRect.width : xW;
		landMarkRect.height = landMarkRect.height > yH ? landMarkRect.height : yH;
	}
	landMarkRect.width -= landMarkRect.x;
	landMarkRect.height -= landMarkRect.y;

	landMarkRect = rectCenterScale(landMarkRect, 1.2);

	landMarkRect = landMarkRect & cv::Rect(0, 0, oriImage.cols, oriImage.rows);
	return landMarkRect;
}

/**
 * @brief 					Get blurness of roi.
 * @param[in] oriImage		Original image.
 * @param[in] faceRect 		The rectangle with absolute ordinates to oriImage.
 * 							All landmarks in this rectangle.
 * @param[in] landmarks		Float array contains landmarks.
 * 							Relative ordinates to @see faceRect.
 * 							Format as [x, x, x, ..., y, y, y, ...].
 * @param[in] landmarksLen	Number of landmarks.
 * @return 					Score of blurness.
 * 		   					The smaller score means more blur.
 * @note					The magic number 32 * 32 is for normalizing
 * 							the number of pixels when calculating.
 * 							In cv::resize(), cv::INTER_NEAREST maybe not
 * 							the best choice.
 */
float getBlurScore(const cv::Mat &oriImage, const cv::Rect &faceRect,
				float* landmarks, int landmarksLen) {
	cv::Rect landMarkRect = getRoi(oriImage, faceRect, landmarks, landmarksLen);
	if (0 >= landMarkRect.area()) {
		return 0;
	}

	cv::Mat faceRoi = oriImage(landMarkRect);
    float rtv = 0;
    cv::Mat blurGray;
    cv::Mat blurAnalysis;
    cv::cvtColor(faceRoi, blurGray, cv::COLOR_BGR2GRAY);

    float expected_pixels = 32 * 32;
    if ((faceRoi.cols * faceRoi.rows) > expected_pixels) {
        float ratio = sqrt(expected_pixels / (faceRoi.cols * faceRoi.rows));
        cv::resize(blurGray, blurGray, cv::Size(0, 0), ratio, ratio, cv::INTER_NEAREST);
    }

    std::cout << "blurGray.width: " << blurGray.cols << std::endl;
    std::cout << "blurGray.height: " << blurGray.rows << std::endl;
    cv::Laplacian(blurGray, blurAnalysis, CV_64F);

    cv::Mat mean;
    cv::Mat stddev;
    cv::meanStdDev(blurAnalysis, mean, stddev);
    rtv = stddev.at<double>(0);
    std::cout << "BlurScore: " << rtv << std::endl;

    // cv::normalize(blurAnalysis, blurAnalysis, 0, 1, cv::NORM_MINMAX);
    // cv::imshow("blurAnalysis", blurAnalysis);
    // cv::imshow("crop4", faceRoi);
    // cv::waitKey();
    return rtv;
}