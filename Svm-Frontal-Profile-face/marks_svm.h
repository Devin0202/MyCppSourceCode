/**
 * @file
 * @author	Devin dai
 * @version	0.0.1
 * @date	2020/05/29
 * @brief	Toolkit for distinguishing frontal and profile faces.
 * @details	Training and using svm to distinguish frontal and
 *          profile facest based on opencv.
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

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <opencv2/opencv.hpp>

/**
 * @brief Data struct for svm training.
 */
struct svmData {
    /// Label for 2-classes: 1, -1.
    cv::Mat label;
    /// Numerical vectors for each entry.
    cv::Mat feature;
};

/**
 * @brief Distinguish frontal and profile face by wrapping cv::ml::SVM.
 */
class DivideFP {
    public:
        /// No default constructor.
        DivideFP() = delete;
        /**
         * @brief			    Constructor.
         * @param[in] loadFile 	Trained model.
         * @code
         * DivideFP mySvm("svm0514.xml");
         * @endcode
         */
        DivideFP(std::string loadFile);
        /**
         * @brief			    Svm prediction.
         * @param[in] faceMarks One feature data.
         * @return			    Svm score.
         *                      In this case, negative value means profile face.
         */
        float mPredict(std::vector<cv::Point2f> &faceMarks);
        /**
         * @brief			    Svm training.
         * @param[in] posData 	Positive data file.
         * @param[in] negData   Negative data file.
         * @param[in] saveModel Destination to save svm model.
         * @return			    Void.
         * @code
         * DivideFP::trainMarks("tmp/test_frontal.txt",
         *                      "tmp/test_profile.txt",
         *                      "tmp/svm.xml");
         * @endcode
         */
        static void trainMarks(std::string posData, std::string negData,
                        std::string saveModel);
    private:
        /// Instance for svm.
        cv::Ptr<cv::ml::SVM> mSvm = nullptr;
        /**
         * @brief			        Normalizing the feature data into [0, 1].
         * @param[in, out] marks 	Feature data.
         * @return                  Void.
         */
        void mNormMarks(std::vector<cv::Point2f> &marks);
        /**
         * @brief			Convert parameter into setting range [lower, upper].
         * @param[in] n 	Parameter.
         * @param[in] lower Down-boundary.
         * @param[in] upper Up-boundary.
         * @return          Value after caluculating.
         */
        float mClip(float n, float lower, float upper);
        /**
         * @brief			    Loading training data.
         * @param[in] posData 	Positive data file.
         * @param[in] negData   Negative data file.
         * @return              Make strings into designed struct.
         *                      @see svmData.
         */
        static svmData dataLoad(std::string posData, std::string negData);
};

/**
 * @brief Useful toolkit for image processing by opencv.
 */
class CvTool {
    public:
        /**
         * @brief			        Normalizing the data into [0, 1].
         * @param[in, out] marks 	Inputting data.
         * @return                  Void.
         */
        static void normMarks(std::vector<cv::Point2f> &marks);
        /**
         * @brief			Convert parameter into setting range [lower, upper].
         * @param[in] n 	Parameter.
         * @param[in] lower Down-boundary.
         * @param[in] upper Up-boundary.
         * @return          Value after caluculating.
         */
        static float clip(float n, float lower, float upper);
        /**
         * @brief			Expanding rectangle with fixed center.
         * @param[in] rect 	Original rectangle @see cv::Rect.
         * @param[in] ratio Expanding ratio to each edge.
         * @return			Expanded rectangle.
         */
        static void rectCenterScale(cv::Rect &rect, float ratio);
        /**
         * @brief			    Converting rectangle into square
         *                      with fixed center.
         * @param[in, out] rect Inputting rectangle.
         * @return              Void.
         * @note                The edge of square is the shorter one
         *                      between width and height from rectangle.
         */
        static void CenterSquare(cv::Rect &rect);
};

/**
 * @brief File processing toolkit.
 */
class CommonTool {
    public:
        /**
         * @brief			    Converting a string into floats
         *                      by seperator.
         * @param[in] src 	    Inputting string.
         * @param[out] rtv      Save floats in std::vector<float>.
         * @param[in] pattern   Seperator used in Inputting string.
         * @return              Void.
         * @code
         * std::vector<float> tmp;
         * CommonTool::splitFloat(s, tmp, " ");
         * @endcode
         */
        static void splitFloat(const std::string& src, std::vector<float>& rtv,
                                const std::string& pattern);
        /**
         * @brief			    Loading file line by line.
         * @param[in] file 	    Path to read file.
         * @return              Save lines in std::vector<std::string>.
         */
        static std::vector<std::string> readTxt(const std::string file);
        /**
         * @brief			    Recording data into file.
         * @param[in] file 	    Path to write file.
         * @param[in] marks     Data for recording.
         * @param[in] needFlip  Both recording the float data x and (1 - x).
         * @return              Void.
         */
        static void writeData(std::string file,
                            std::vector<cv::Point2f> marks,
                            bool needFlip = false);
};