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

# pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

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
				float* landmarks, int landmarksLen);
