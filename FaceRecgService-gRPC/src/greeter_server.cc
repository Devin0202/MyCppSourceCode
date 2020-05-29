/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "FaceRecg.grpc.pb.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>

#include "interface_face_recognizer.h"

#include <signal.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using facerecg::LogRequest;
using facerecg::LogReply;
using facerecg::FeatureRequest;
using facerecg::FeatureReply;
using facerecg::Frecg;
using facerecg::AbsRect;
using facerecg::DetectRequest;
using facerecg::CmpFeatureRequest;
using facerecg::CmpFeatureReply;
using facerecg::CmpImageRequest;
using facerecg::CmpImageReply;
using facerecg::QualityRequest;
using facerecg::QualityReply;

#define VERSION  "1.0.0.8"

std::unique_ptr<Server> server;

void sigint_handler(int sig) {
    if (SIGINT == sig) {
        server->Shutdown();
        std::cout << "\nctrl+c pressed!" << std::endl;
    }
    return;
}
// Logic and data behind the server's behavior.
class FrServiceImpl final : public Frecg::Service {
    Status logIn(ServerContext* context, const LogRequest* request,
                    LogReply* reply) override {
        std::cout << "Someone use logIn~" << std::endl;
        std::string prefix("Hello ");
        reply->set_message(prefix + request->message());
        reply->set_algversion(VERSION);
        return Status::OK;
    }

    Status compareFeature(ServerContext* context,
                        const CmpFeatureRequest* request,
                        CmpFeatureReply* reply) override {
        reply->set_message("In compareFeature");
        if (0 != request->featurea().size()
            && request->featurea().size() == request->featureb().size()) {
            float *faceA = new float[request->featurea().size()];
            float *faceB = new float[request->featureb().size()];

            for (int i = 0; i < request->featurea().size(); i++) {
                faceA[i] = request->featurea(i);
                faceB[i] = request->featureb(i);
            }

            float resemblance =
                HiarFace_compareFaceFeature(faceA, request->featurea().size(),
                                            faceB, request->featureb().size());

            delete[] faceA;
            delete[] faceB;
            faceA = nullptr;
            faceB = nullptr;
            reply->set_resemblance(resemblance);
        } else {
            reply->set_resemblance(0);
            reply->set_message("In compareFeature, two lengthes dismatched!");
        }
        return Status::OK;
    }

    Status compareImage(ServerContext* context,
                        const CmpImageRequest* request,
                        CmpImageReply* reply) override {
        reply->set_message("In compareImage");
        float *featureA = nullptr;
        float *featureB = nullptr;
        int len_features;
        int ret = 0;
        if (request->has_recta() && request->has_rectb()) {
            int face_bboxesA[4] = {request->recta().left(),
                                    request->recta().top(),
                                    request->recta().width(),
                                    request->recta().height()};
            int face_bboxesB[4] = {request->rectb().left(),
                                    request->rectb().top(),
                                    request->rectb().width(),
                                    request->rectb().height()};

            ret = HiarFace_extractFeature(
                                (unsigned char *)request->imagedataa().c_str(),
                                request->imagedataa().size(),
                                face_bboxesA,
                                1,
                                &featureA,
                                &len_features);

            // std::cout << face_bboxesB[0] << face_bboxesB[1]
            //         << face_bboxesB[2] << face_bboxesB[3] << std::endl;
            // saveImage(request->imagedatab().c_str(), request->imagedatab().size());
            ret = HiarFace_extractFeature(
                                (unsigned char *)request->imagedatab().c_str(),
                                request->imagedatab().size(),
                                face_bboxesB,
                                1,
                                &featureB,
                                &len_features);
            // std::cout << face_bboxesA[0] << face_bboxesA[1]
            //         << face_bboxesA[2] << face_bboxesA[3] << std::endl;

            float resemblance = HiarFace_compareFaceFeature(featureA,
                                                        HIAR_FACE_FEATURE_LEN,
                                                        featureB,
                                                        HIAR_FACE_FEATURE_LEN);
            delete[] featureA;
            delete[] featureB;
            reply->set_resemblance(resemblance);
        } else {
            reply->set_resemblance(0);
            reply->set_message("In compareImage: Rois missing!!!");
        }
        return Status::OK;
    }

    Status getFaceQuality(ServerContext* context, const QualityRequest* request,
                    QualityReply* reply) override {
        reply->set_message("In getFaceQuality");
        int num_bbox = request->rects().size();
        if (0 >= num_bbox) {
            reply->set_message("In getFaceQuality: No rois!!!");
            return Status::OK;
        }

        int *face_quality = new int[num_bbox];
        float *face_direction = new float[num_bbox];
        int *face_bboxes = new int[4 * num_bbox];
        for (int i = 0; i < num_bbox; i++) {
            face_bboxes[4 * i] = request->rects(i).left();
            face_bboxes[4 * i + 1] = request->rects(i).top();
            face_bboxes[4 * i + 2] = request->rects(i).width();
            face_bboxes[4 * i + 3] = request->rects(i).height();
        }

        int ret = HiarFace_getQualityFaceCrops(
                        (unsigned char *)request->imagedata().c_str(),
                        request->imagedata().size(),
                        face_bboxes,
                        num_bbox,
                        face_quality,
                        face_direction);
        if (nullptr != face_quality) {
            for (int i = 0; i < num_bbox; i++) {
                reply->add_quality(face_quality[i]);
            }
            delete[] face_quality;
            face_quality = nullptr;
        }
        if (nullptr != face_direction) {
            for (int i = 0; i < num_bbox; i++) {
                reply->add_isfrontal(face_direction[i]);
            }
            delete[] face_direction;
            face_direction = nullptr;
        }
        if (nullptr != face_bboxes) {
            delete[] face_bboxes;
            face_bboxes = nullptr;
        }
        return Status::OK;
    }

    Status featureExtract(ServerContext* context, const FeatureRequest* request,
                    FeatureReply* reply) override {
        reply->set_message("In featureExtract");

        extract_feature((unsigned char *)request->imagedata().c_str(),
                        request->imagedata().size(),
                        reply,
                        false,
                        request);
        // std::cout << reply->rects().size() << ' '
        //         << reply->features().size() << std::endl;
        // std::string rtvS = saveImage(request->imagedata().c_str(),
        //                             request->imagedata().size());
        return Status::OK;
    }

    Status featureDetect(ServerContext* context, const DetectRequest* request,
                    FeatureReply* reply) override {
        // std::string rtvS = saveImage(request->imagedata().c_str(),
        //                                 request->imagedata().size());
        reply->set_message("In featureDetect");

        extract_feature((unsigned char *)request->imagedata().c_str(),
                        request->imagedata().size(),
                        reply,
                        true,
                        nullptr);
        // std::cout << reply->rects().size() << ' '
        //         << reply->features().size() << std::endl;
        return Status::OK;
    }

    public:
        std::string imagesSaver = "./";
        FrServiceImpl(std::string folder) {
            imagesSaver = folder;
        }

    private:
        std::string saveImage(const char *dataImage,
                                const int lenImage) {
            /**
             * Convert bytes flow into cv::Mat.
             */
            std::vector<uchar> vecData(dataImage, dataImage + lenImage);
            cv::Mat image = cv::imdecode(vecData, CV_LOAD_IMAGE_COLOR);

            if (image.empty()) {
                std::cout << "Image is empty!!!" << std::endl;
                return "Image is empty!!!";
            } else {
                std::time_t result = std::time(NULL);
                std::string saveFile =
                            std::to_string(::localtime(&result)->tm_year + 1900)
                            + '-' + std::to_string(::localtime(&result)->tm_mon + 1)
                            + '-' + std::to_string(::localtime(&result)->tm_mday)
                            + '-' + std::to_string(::localtime(&result)->tm_hour)
                            + '-' + std::to_string(::localtime(&result)->tm_min)
                            + '-' + std::to_string(::localtime(&result)->tm_sec);

                if (cv::imwrite(this->imagesSaver + saveFile + ".jpg", image)) {
                    std::cout << "Someone use save image~" << std::endl;
                } else {
                    std::cout << "cv::imwrite failed!!!" << std::endl;
                }
                return "Image H: " + std::to_string(image.rows)
                        + ' ' + "Image W: " + std::to_string(image.cols);
            }
        }

        void extract_feature(const unsigned char *dataImage,
                            const int lenImage, 
                            FeatureReply* reply,
                            bool needDetect,
                            const FeatureRequest* request) {
            int *face_bboxes = nullptr;
            int num_bbox;
            float *feature = nullptr;
            int len_features;
            int ret =  0;
            if (needDetect && nullptr == request) {
                ret = HiarFace_detectAndExtractFeature(dataImage, lenImage,
                            &face_bboxes, &num_bbox, &feature, &len_features);
            } else {
                num_bbox = request->rects().size();
                face_bboxes = new int[4 * num_bbox];
                for (int i = 0; i < num_bbox; i++) {
                    face_bboxes[4 * i] = request->rects(i).left();
                    face_bboxes[4 * i + 1] = request->rects(i).top();
                    face_bboxes[4 * i + 2] = request->rects(i).width();
                    face_bboxes[4 * i + 3] = request->rects(i).height();
                }
                // std::cout << "num_bbox!!!" << num_bbox << std::endl;
                ret = HiarFace_extractFeature(dataImage, lenImage,
                        face_bboxes, num_bbox, &feature, &len_features);
            }

            if (ret != 1 || num_bbox < 1) {
                std::cout << "No face !!!" << std::endl;
                if (face_bboxes != nullptr) {
                    delete[] face_bboxes;
                    face_bboxes = nullptr;
                }

                if (feature != nullptr) {
                    delete[] feature;
                    feature = nullptr;
                }
                return;
            }

            if (len_features != num_bbox * HIAR_FACE_FEATURE_LEN) {
                std::cout << "feature of face boxes are Error!" << std::endl;
                if (face_bboxes != nullptr) {
                    delete[] face_bboxes;
                    face_bboxes = nullptr;
                }

                if (feature != nullptr) {
                    delete[] feature;
                    feature = nullptr;
                }
                return;
            }
            
            int *face_quality = new int[num_bbox];
            float *face_direction = new float[num_bbox];
            HiarFace_getQualityFaceCrops(dataImage, lenImage, face_bboxes,
                                        num_bbox, face_quality, face_direction);
            if (nullptr != face_quality) {
                for (int i = 0; i < num_bbox; i++) {
                    if (face_quality[i] < 5
                        || face_direction[i] <= 0) {
                        std::cout << "Low quality occured!!!" << std::endl
                                    << "LTWH: "
                                    << request->rects(i).left() << ' '
                                    << request->rects(i).top() << ' '
                                    << request->rects(i).width() << ' '
                                    << request->rects(i).height() << std::endl;
                        continue;
                    }
                    auto rctface = reply->add_rects();
                    rctface->set_left(face_bboxes[i * 4]);
                    rctface->set_top(face_bboxes[i * 4 + 1]);
                    rctface->set_width(face_bboxes[i * 4 + 2]);
                    rctface->set_height(face_bboxes[i * 4 + 3]);

                    for (int j = 0; j < HIAR_FACE_FEATURE_LEN; j++) {
                        reply->add_features(
                            feature[i * HIAR_FACE_FEATURE_LEN + j]);
                    }
                }

                delete[] face_quality;
                face_quality = nullptr;
                delete[] face_direction;
                face_direction = nullptr;
            }

            if (face_bboxes != nullptr) {
                delete[] face_bboxes;
                face_bboxes = nullptr;
            }

            if (feature != nullptr) {
                delete[] feature;
                feature = nullptr;
            }
            return;
        }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    FrServiceImpl service("tmp/");

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    server = std::unique_ptr<Server>(builder.BuildAndStart());
    std::cout << "Server listening ON " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

int main(int argc, char** argv) {
    signal(SIGINT, sigint_handler);
    //设置logger
    std::string logger_path = "logs/log.txt";
    // 模型文件路径，该路径下的mcnn文件夹下是人脸检测模型，r50文件夹下是人脸识别模型
    std::string model_path="../models";

    std::cout << logger_path << " " << model_path << std::endl;

    int ret = HiarFace_initRecognizer(model_path.c_str(), logger_path.c_str());

    if (1 != ret) {
        std::cout << "initial Recognizer is failure!" << std::endl;
        return 0;
    }
    
    RunServer();

    HiarFace_releaseRecognizer();
    std::cout << "Server shutdown~" << std::endl;
    return 0;
}