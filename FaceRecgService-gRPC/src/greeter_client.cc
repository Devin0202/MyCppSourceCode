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
#include <fstream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "FaceRecg.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using facerecg::LogRequest;
using facerecg::LogReply;
using facerecg::FeatureRequest;
using facerecg::FeatureReply;
using facerecg::Frecg;
using facerecg::DetectRequest;
using facerecg::CmpFeatureRequest;
using facerecg::CmpFeatureReply;
using facerecg::CmpImageRequest;
using facerecg::CmpImageReply;
using facerecg::QualityRequest;
using facerecg::QualityReply;

using facerecg::AbsRect;

class FrClient {
    public:
        FrClient(std::shared_ptr<Channel> channel)
                : stub_(Frecg::NewStub(channel)) {}
        // Assembles the client's payload, sends it and presents the response back from the server.
        std::string logIn(const std::string& user) {
        // Data we are sending to the server.
            LogRequest request;
            request.set_message(user);
            // Container for the data we expect from the server.
            LogReply reply;
            // Context for the client. It could be used to convey extra information to the server and/or tweak certain RPC behaviors.
            ClientContext context;
            // The actual RPC.
            Status status = stub_->logIn(&context, request, &reply);
            // Act upon its status.
            if (status.ok()) {
                std::cout << "Greeter received: " << reply.message()
                            << std::endl;
                return reply.algversion();
            } else {
                std::cout << status.error_code() << ": "
                            << status.error_message()
                            << std::endl;
                return "RPC failed";
            }
        }

        std::string getFaceQuality(const std::string& info,
                                const std::vector<std::vector<int>> rois) {
            if (0 >= rois.size()) {
                return "No rois!!!";
            }
            // Data we are sending to the server.
            QualityRequest request;
            for (auto roi : rois) {
                auto tmp = request.add_rects();
                tmp->set_left(roi.at(0));
                tmp->set_top(roi.at(1));
                tmp->set_width(roi.at(2));
                tmp->set_height(roi.at(3));
            }
            /**
             * Load image file as bytes flow.
             */
            std::ifstream is(info, std::ifstream::in | std::ifstream::binary);
            if (is) {
                is.seekg(0, is.end);
                int length = is.tellg();
                is.seekg(0, is.beg);
                char *buffer = new char[length];
                is.read(buffer, length);
                request.set_imagedata(buffer, length);
                delete [] buffer;
            } else {
                return "Image loading error: " + info;
            }
            request.set_message(info);
            // Container for the data we expect from the server.
            QualityReply reply;
            // Context for the client. It could be used to convey extra information to the server and/or tweak certain RPC behaviors.
            ClientContext context;
            // The actual RPC.
            Status status = stub_->getFaceQuality(&context, request, &reply);
            // Act upon its status.
            if (status.ok()) {
                // std::ofstream f1("./tmp/quality.txt");
                // if (f1) {
                //     f1 << "Quality" << '\t' << "IsFrontal" << std::endl;
                //     for (int i = 0; i < reply.quality().size(); i++) {
                //         f1 << reply.quality(i) << "\t\t"
                //             << reply.isfrontal(i) << std::endl;
                //     }
                //     f1.close();
                // }
                return reply.message();
            } else {
                std::cout << status.error_code() << ": "
                            << status.error_message()
                            << std::endl;
                return "RPC failed";
            }
        }

        std::string featureExtract(const std::string& info,
                                const std::vector<std::vector<int>> rois) {
            // Data we are sending to the server.
            FeatureRequest request;
            for (auto roi : rois) {
                auto tmp = request.add_rects();
                tmp->set_left(roi.at(0));
                tmp->set_top(roi.at(1));
                tmp->set_width(roi.at(2));
                tmp->set_height(roi.at(3));
            }
            /**
             * Load image file as bytes flow.
             */
            std::ifstream is(info, std::ifstream::in | std::ifstream::binary);
            if (is) {
                is.seekg(0, is.end);
                int length = is.tellg();
                is.seekg(0, is.beg);
                char *buffer = new char[length];
                is.read(buffer, length);
                request.set_imagedata(buffer, length);
                delete [] buffer;
            } else {
                return "Image loading error: " + info;
            }
            request.set_message(info);
            // Container for the data we expect from the server.
            FeatureReply reply;
            // Context for the client. It could be used to convey extra information to the server and/or tweak certain RPC behaviors.
            ClientContext context;
            // The actual RPC.
            Status status = stub_->featureExtract(&context, request, &reply);
            // Act upon its status.
            if (status.ok()) {
                // std::cout << "valid roi number: " << reply.rects().size()
                //             << std::endl;
                // std::cout << reply.rects(0).left() << ' '
                //         << reply.rects(0).top() << ' '
                //         << reply.rects(0).width() << ' '
                //         << reply.rects(0).height() << std::endl;
                // std::ofstream f1("./tmp/2.txt");
                // if (f1) {
                //     for (int i = 0; i < reply.features().size(); i++) {
                //         f1 << reply.features(i) << std::endl;
                //     }
                //     f1.close();
                // }
                return reply.message();
            } else {
                std::cout << status.error_code() << ": "
                            << status.error_message()
                            << std::endl;
                return "RPC failed";
            }
        }

        std::string featureDetect(const std::string& info) {
            // Data we are sending to the server.
            DetectRequest request;
            /**
             * Load image file as bytes flow.
             */
            std::ifstream is(info, std::ifstream::in | std::ifstream::binary);
            if (is) {
                is.seekg(0, is.end);
                int length = is.tellg();
                is.seekg(0, is.beg);
                char *buffer = new char[length];
                is.read(buffer, length);
                request.set_imagedata(buffer, length);
                delete [] buffer;
                is.close();
            } else {
                return "Image loading error: " + info;
            }
            request.set_message(info);
            // Container for the data we expect from the server.
            FeatureReply reply;
            // Context for the client. It could be used to convey extra information to the server and/or tweak certain RPC behaviors.
            ClientContext context;
            // The actual RPC.
            Status status = stub_->featureDetect(&context, request, &reply);
            // Act upon its status.
            if (status.ok()) {
                std::cout << reply.rects(0).left() << ' '
                        << reply.rects(0).top() << ' '
                        << reply.rects(0).width() << ' '
                        << reply.rects(0).height() << std::endl;
                // std::ofstream f1("./tmp/1.txt");
                // if (f1) {
                //     for (int i = 0; i < reply.features().size(); i++) {
                //         f1 << reply.features(i) << std::endl;
                //     }
                //     f1.close();
                // }
                return reply.message();
            } else {
                std::cout << status.error_code() << ": "
                            << status.error_message()
                            << std::endl;
                return "RPC failed";
            }
        }

        std::string compareFeature(const std::vector<float> &cmpA,
                                    const std::vector<float> &cmpB) {
            // Data we are sending to the server.
            CmpFeatureRequest request;
            for (auto i : cmpA) {
                request.add_featurea(i);
            }
            for (auto i : cmpB) {
                request.add_featureb(i);
            }
            // Container for the data we expect from the server.
            CmpFeatureReply reply;
            // Context for the client. It could be used to convey extra information to the server and/or tweak certain RPC behaviors.
            ClientContext context;
            // The actual RPC.
            Status status = stub_->compareFeature(&context, request, &reply);
            // Act upon its status.
            if (status.ok()) {
                std::cout << "Resemblance: " << reply.resemblance()
                        << std::endl;
                return reply.message();
            } else {
                std::cout << status.error_code() << ": "
                            << status.error_message()
                            << std::endl;
                return "RPC failed";
            }
        }

        std::string compareImage(const std::string& imgA,
                                const std::string& imgB,
                                const std::vector<int>& roiA,
                                const std::vector<int>& roiB) {
            // Data we are sending to the server.
            CmpImageRequest request;
            /**
             * Load image files as bytes flow.
             */
            if (4 == roiA.size() && 4 == roiB.size()) {
                request.mutable_recta()->set_left(roiA.at(0));
                request.mutable_recta()->set_top(roiA.at(1));
                request.mutable_recta()->set_width(roiA.at(2));
                request.mutable_recta()->set_height(roiA.at(3));
                request.mutable_rectb()->set_left(roiB.at(0));
                request.mutable_rectb()->set_top(roiB.at(1));
                request.mutable_rectb()->set_width(roiB.at(2));
                request.mutable_rectb()->set_height(roiB.at(3));
            } else {
                return "Roi info error!!!";
            }

            std::ifstream is(imgA, std::ifstream::in | std::ifstream::binary);
            if (is) {
                is.seekg(0, is.end);
                int length = is.tellg();
                is.seekg(0, is.beg);
                char *buffer = new char[length];
                is.read(buffer, length);
                request.set_imagedataa(buffer, length);
                delete[] buffer;
                is.close();
            } else {
                return "Image loading error: " + imgA;
            }
            is.open(imgB, std::ifstream::in | std::ifstream::binary);
            if (is) {
                is.seekg(0, is.end);
                int length = is.tellg();
                is.seekg(0, is.beg);
                char *buffer = new char[length];
                is.read(buffer, length);
                request.set_imagedatab(buffer, length);
                delete[] buffer;
                is.close();
            } else {
                return "Image loading error: " + imgB;
            }
            // Container for the data we expect from the server.
            CmpImageReply reply;
            // Context for the client. It could be used to convey extra information to the server and/or tweak certain RPC behaviors.
            ClientContext context;
            // The actual RPC.
            Status status = stub_->compareImage(&context, request, &reply);
            // Act upon its status.
            if (status.ok()) {
                std::cout << "Resemblance: " << reply.resemblance()
                        << std::endl;
                return reply.message();
            } else {
                std::cout << status.error_code() << ": "
                            << status.error_message()
                            << std::endl;
                return "RPC failed";
            }
        }
    private:
        std::unique_ptr<Frecg::Stub> stub_;
};

int main(int argc, char** argv) {
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint specified by
    // the argument "--target=" which is the only expected argument.
    // We indicate that the channel isn't authenticated (use of
    // InsecureChannelCredentials()).
    std::string target_str;
    std::string arg_server("--target");
    std::string image_str;
    std::string arg_image("--image");
    if (3 == argc) {
        std::string arg_val = argv[1];
        size_t start_pos = arg_val.find(arg_server);
        if (start_pos != std::string::npos) {
            start_pos += arg_server.size();
            if (arg_val[start_pos] == '=') {
                target_str = arg_val.substr(start_pos + 1);
            } else {
                std::cout << "The only correct argument syntax is --target="
                            << std::endl;
                return 0;
            }
        } else {
            std::cout << "The only acceptable argument is \
                            --target=XXX --image=YYY"
                        << std::endl;
            return 0;
        }

        arg_val = argv[2];
        start_pos = arg_val.find(arg_image);
        if (start_pos != std::string::npos) {
            start_pos += arg_image.size();
            if (arg_val[start_pos] == '=') {
                image_str = arg_val.substr(start_pos + 1);
            } else {
                std::cout << "The only correct argument syntax is --image="
                            << std::endl;
                return 0;
            }
        } else {
            std::cout << "The only acceptable argument is \
                            --target=XXX --image=YYY"
                        << std::endl;
            return 0;
        }
    } else {
        target_str = "localhost:50051";
        image_str = "test.jpg";
    }
    FrClient greeter(grpc::CreateChannel(target_str,
                        grpc::InsecureChannelCredentials()));
    /**
     *  Call rpc logIn (LogRequest) returns (LogReply) {}
     */
    std::string user("My Lovely World");
    std::string reply = greeter.logIn(user);
    std::cout << "Alg Version: " << reply << std::endl;
    /**
     *  Call rpc featureDetect (DetectRequest) returns (FeatureReply) {}
     */
    reply = greeter.featureDetect(image_str);
    std::cout << "faceRecg: " << reply << std::endl << std::endl;
    // /**
    //  *  Call rpc featureExtract (FeatureRequest) returns (FeatureReply) {}
    //  */
    // std::vector<std::vector<int>> rois;
    // rois.push_back({129, 55, 141, 141});
    // rois.push_back({200, 55, 100, 100});
    // reply = greeter.featureExtract(image_str, rois);
    // std::cout << "faceRecg: " << reply << std::endl << std::endl;
    // /**
    //  *  Call rpc getFaceQuality (QualityRequest) returns (QualityReply) {}
    //  */
    // reply = greeter.getFaceQuality(image_str, rois);
    // std::cout << "faceRecg: " << reply << std::endl << std::endl;
    // /**
    //  *  Call rpc compareFeature (CmpFeatureRequest) returns (CmpFeatureReply) {}
    //  */
    // std::vector<float> cmpA;
    // std::vector<float> cmpB;
    // std::ifstream is("./tmp/3.txt", std::ifstream::in);
    // if (is) {
    //     std::string line;
    //     while (getline (is, line)) {   
    //         cmpA.push_back(std::stof(line));  
    //     }
    //     is.close();
    // } else {
    //     std::cout << "file loading error: " << std::endl;
    // }
    // is.open("./tmp/4.txt", std::ifstream::in);
    // if (is) {
    //     std::string line;
    //     while (getline (is, line)) {   
    //         cmpB.push_back(std::stof(line));  
    //     }
    //     is.close();
    // } else {
    //     std::cout << "file loading error: " << std::endl;
    // }
    // reply = greeter.compareFeature(cmpA, cmpB);
    // std::cout << "faceRecg: " << reply << std::endl << std::endl;
    // /**
    //  *  Call rpc compareImage (CmpImageRequest) returns (CmpImageReply) {}
    //  */
    // std::vector<int> roiA({45, 85, 218, 218});
    // // std::vector<int> roiB({66, 94, 236, 236});
    // std::vector<int> roiB({30, 30, 136, 136});
    // // std::vector<int> roiB({45, 85, 218, 218});
    // reply = greeter.compareImage("./8.jpg", "./9.jpg", roiA, roiB);
    // std::cout << "faceRecg: " << reply << std::endl << std::endl;
    return 0;
}