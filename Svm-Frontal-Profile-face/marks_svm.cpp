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

#include "marks_svm.h"

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
void CommonTool::splitFloat(const std::string& src,
                            std::vector<float>& rtv,
                            const std::string& pattern) {
    std::string::size_type pos1, pos2;
    pos2 = src.find(pattern);
    pos1 = 0;
    while (std::string::npos != pos2) {
        rtv.push_back(std::stof(src.substr(pos1, pos2 - pos1)));
        pos1 = pos2 + pattern.size();
        pos2 = src.find(pattern, pos1);
    }
    if (pos1 != src.length()) {
        rtv.push_back(std::stof(src.substr(pos1)));
    }
    return;
}

/**
 * @brief			    Loading file line by line.
 * @param[in] file 	    Path to read file.
 * @return              Save lines in std::vector<std::string>.
 */
std::vector<std::string> CommonTool::readTxt(const std::string file) {
    std::vector<std::string> rtv;
    std::ifstream infile; 
    infile.open(file.data());
    assert(infile.is_open());

    std::string s;
    while (getline(infile, s)) {
        rtv.push_back(s);
        std::cout << s << std::endl;
    }
    infile.close();
    return rtv;
}

/**
 * @brief			    Recording data into file.
 * @param[in] file 	    Path to write file.
 * @param[in] marks     Data for recording.
 * @param[in] needFlip  Both recording the float data x and (1 - x).
 * @return              Void.
 */
void CommonTool::writeData(std::string file,
                            std::vector<cv::Point2f> marks, bool needFlip) {
    std::ofstream outfile; 
    outfile.open(file.data(), std::ofstream::app);
    assert(outfile.is_open());

    int i = 0;
    for (i = 0; i < int(marks.size()) - 1; i++) {
        outfile << marks.at(i).x << ' ' << marks.at(i).y << ' ';
    }
    outfile << marks.at(i).x << ' ' << marks.at(i).y << std::endl;

    if (needFlip) {
        for (i = 0; i < int(marks.size()) - 1; i++) {
            outfile << 1 - marks.at(i).x << ' ' << marks.at(i).y << ' ';
        }
        outfile << 1 - marks.at(i).x << ' ' << marks.at(i).y << std::endl;
    }

    outfile.close();
    return;
}

/**
 * @brief			    Converting rectangle into square
 *                      with fixed center.
 * @param[in, out] rect Inputting rectangle.
 * @return              Void.
 * @note                The edge of square is the shorter one
 *                      between width and height from rectangle.
 */
void CvTool::CenterSquare(cv::Rect &rect) {
    int diff = rect.width - rect.height;
    if (0 < diff) {
        rect.y -= int(diff / 2);
        rect.height += diff;
    } else {
        diff = -diff;
        rect.x -= int(diff / 2);
        rect.width += diff;  
    }
    return;
}

/**
 * @brief			Expanding rectangle with fixed center.
 * @param[in] rect 	Original rectangle @see cv::Rect.
 * @param[in] ratio Expanding ratio to each edge.
 * @return			Expanded rectangle.
 */
void CvTool::rectCenterScale(cv::Rect &rect, float ratio) {
	if (0 > ratio) {
		return;
	} else {
		int deltaW = cvRound((ratio - 1) * rect.width);
		int deltaH = cvRound((ratio - 1) * rect.height);
		cv::Point pt;
		pt.x = cvRound(deltaW / 2.0);
		pt.y = cvRound(deltaW / 2.0);
		rect.width += deltaW;
		rect.height += deltaH;
		return;
	}
}

/**
 * @brief			        Normalizing the data into [0, 1].
 * @param[in, out] marks 	Inputting data.
 * @return                  Void.
 */
void CvTool::normMarks(std::vector<cv::Point2f> &marks) {
    assert(0 < marks.size());

    float xMn = marks.at(0).x;
    float xMx = marks.at(0).x;
    float yMn = marks.at(0).y;
    float yMx = marks.at(0).y;
    for (auto i : marks) {
        xMn = i.x < xMn ? i.x : xMn;
        xMx = i.x > xMx ? i.x : xMx;
        yMn = i.y < yMn ? i.y : yMn;
        yMx = i.y > yMx ? i.y : yMx;
    }

    float xSpan = xMx - xMn;
    float ySpan = yMx - yMn;
    for (int i = 0; i < int(marks.size()); i++) {
        marks.at(i).x = clip((marks.at(i).x - xMn) / xSpan, 0.0f, 1.0f);
        marks.at(i).y = clip((marks.at(i).y - yMn) / ySpan, 0.0f, 1.0f);
    }
    return;
}

/**
 * @brief			Convert parameter into setting range [lower, upper].
 * @param[in] n 	Parameter.
 * @param[in] lower Down-boundary.
 * @param[in] upper Up-boundary.
 * @return          Value after caluculating.
 */
float CvTool::clip(float n, float lower, float upper) {
  return std::max(lower, std::min(n, upper));
}

/**
 * @brief			    Loading training data.
 * @param[in] posData 	Positive data file.
 * @param[in] negData   Negative data file.
 * @return              Make strings into designed struct.
 *                      @see svmData.
 */
svmData DivideFP::dataLoad(std::string posData, std::string negData) {
    svmData out;
    cv::Mat trainingDataMat;
    std::string s;
    std::ifstream infile; 

    infile.open(posData.data());
    assert(infile.is_open());
    while (getline(infile, s)) {
        std::vector<float> tmp;
        CommonTool::splitFloat(s, tmp, " ");
        cv::Mat tmpMat(tmp);

        if (trainingDataMat.empty()) {
            trainingDataMat = tmpMat.t();
        } else {
            cv::vconcat(trainingDataMat, tmpMat.t(), trainingDataMat);
        }
    }
    infile.close();
    cv::Mat labelsMat(trainingDataMat.rows, 1, CV_32SC1, 1);

    infile.open(negData.data());
    assert(infile.is_open());
    while (getline(infile, s)) {
        std::vector<float> tmp;
        CommonTool::splitFloat(s, tmp, " ");
        cv::Mat tmpMat(tmp);

        if (trainingDataMat.empty()) {
            trainingDataMat = tmpMat.t();
        } else {
            cv::vconcat(trainingDataMat, tmpMat.t(), trainingDataMat);
        }
    }
    infile.close();
    cv::vconcat(labelsMat, cv::Mat(trainingDataMat.rows - labelsMat.rows, 1, CV_32SC1, -1), labelsMat);

    out.feature = trainingDataMat;
    out.label = labelsMat;
    return out;
}

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
void DivideFP::trainMarks(std::string posData, std::string negData,
                std::string saveModel) {
    svmData trainSet = dataLoad(posData, negData);
    // std::cout << trainSet.label.at<int>(0, 0) << std::endl;
    // std::cout << trainSet.feature.at<float>(0, 0) << std::endl;

    cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
    svm->setType(cv::ml::SVM::C_SVC);
    svm->setKernel(cv::ml::SVM::RBF);
    svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER,
                                            1000, 1e-6));

    svm->train(trainSet.feature, cv::ml::ROW_SAMPLE, trainSet.label);
    svm->save(saveModel);
    return;
}

/**
 * @brief			    Constructor.
 * @param[in] loadFile 	Trained model.
 * @code
 * DivideFP mySvm("svm0514.xml");
 * @endcode
 */
DivideFP::DivideFP(std::string loadFile) {
    assert(-1 != access(loadFile.c_str(), R_OK));
    mSvm = cv::ml::StatModel::load<cv::ml::SVM>(loadFile);
    assert(!mSvm.empty());
    return;
}

/**
 * @brief			        Normalizing the feature data into [0, 1].
 * @param[in, out] marks 	Feature data.
 * @return                  Void.
 */
void DivideFP::mNormMarks(std::vector<cv::Point2f> &marks) {
    assert(0 < marks.size());

    float xMn = marks.at(0).x;
    float xMx = marks.at(0).x;
    float yMn = marks.at(0).y;
    float yMx = marks.at(0).y;
    for (auto i : marks) {
        xMn = i.x < xMn ? i.x : xMn;
        xMx = i.x > xMx ? i.x : xMx;
        yMn = i.y < yMn ? i.y : yMn;
        yMx = i.y > yMx ? i.y : yMx;
    }

    float xSpan = xMx - xMn;
    float ySpan = yMx - yMn;
    for (int i = 0; i < int(marks.size()); i++) {
        marks.at(i).x = mClip((marks.at(i).x - xMn) / xSpan, 0.0f, 1.0f);
        marks.at(i).y = mClip((marks.at(i).y - yMn) / ySpan, 0.0f, 1.0f);
    }
    return;
}

/**
 * @brief			    Svm prediction.
 * @param[in] faceMarks One feature data.
 * @return			    Svm score.
 *                      In this case, negative value means profile face.
 */
float DivideFP::mPredict(std::vector<cv::Point2f> &faceMarks) {
    assert(!mSvm.empty());
    mNormMarks(faceMarks);
    std::vector<float> convert;
    for (int i = 0; i < int(faceMarks.size()); i++) {
        convert.push_back(faceMarks.at(i).x);
        convert.push_back(faceMarks.at(i).y);
    }
    cv::Mat tmpMat(convert);
    tmpMat = tmpMat.t();
    return mSvm->predict(tmpMat);
}

/**
 * @brief			Convert parameter into setting range [lower, upper].
 * @param[in] n 	Parameter.
 * @param[in] lower Down-boundary.
 * @param[in] upper Up-boundary.
 * @return          Value after caluculating.
 */
float DivideFP::mClip(float n, float lower, float upper) {
    return std::max(lower, std::min(n, upper));
};