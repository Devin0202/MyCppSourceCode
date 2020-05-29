#pragma once
/******************************************************************************************
* interface_face_recognizer.h
*
* Copyright 2020, HiScene
*
* This file is HiScene's property. It contains HiScene's proprietary and confidential
* information and trade secret. The information and code contained in this file is only
* intended for authorized HiScene employees and customers.
*
* DO NOT DISTRIBUTE, DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION FROM
* HISCENE.
*
* If you are not an intended recipient of this file, you must not copy, distribute, modify,
* or take any action in reliance on it. If you have received this file in error, please
* immediately notify HiScene, and permanently delete the original and any copy of it and
* any printout thereof.
*
******************************************************************************************/
// Document format: Doxygen

#include <cstdint>                  // uint16_t, int32_t, int64_t
#include <cstddef>                  // size_t
#include <cassert>                  // assert
#include <type_traits>              // std::is_signed, is_integral, is_same, underlying_type

/** @file */        // to generate doxygen for global function, typedef, enum or preprocessor definition 
                    // http://stackoverflow.com/questions/12041274/doxygen-not-documenting-main-function-in-main-cpp
/*************************************************************************************************************
**************************************************************************************************************
    HIAR_API      API export convention for exposing module symbols to external users
    HIAR_CALL     Call convention for function

    The macros above are used in HiAR to control exporting and calling convention of library
    objects.

    HIAR_API:
      1). If you are building a DLL/so of HiScene Foundations, define this macro in the project scope:
            HIAR_EXPORTS
          All the symbols decorated with HIAR_API will be properly exported in the DLL/so.

      2). If you are building a project using the DLL/so built above, define this macro in the project scope
          (before your code includes this header file):
            HIAR_IMPORTS
          All the symbols decorated with HIAR_API will be properly imported from the DLL/so.

      3). If you are building or using a static library in HiScene Foundations, do not define any of these:
            HIAR_EXPORTS
            HIAR_IMPORTS

    HIAR_CALL:
      This is only used by certain library code that needs explicit control.

    The macros above are mostly for Windows/Visual C++. On other platforms they are probably empty.

Example usage:
    HIAR_API void foo();                        // export a function
    class HIAR_API Foo{};                       // export a class
    void HIAR_CALL bar();                       // calling convention of a function
    HIAR_API void HIAR_CALL hey();              // export a function with an explicit calling convention
    void (HIAR_CALL * pfn)(const char* msg);    // a function pointer with an explicit calling convention

*************************************************************************************************************/

#undef HIAR_API
#undef HIAR_CALL

/*************************************************************************************************************
**************************************************************************************************************
    HIAR_API
*************************************************************************************************************/

#if defined(_WIN32)
// on Windows: DLL handling can be more efficient here
#if defined( HIAR_EXPORTS )   // when building DLLs, module projects define this macro
#define HIAR_API __declspec(dllexport)
#elif defined( HIAR_IMPORTS ) // when using DLLs, client projects define this macro
#define HIAR_API __declspec(dllimport)
#else // when building or using target static library, or whatever: define it as nothing
#define HIAR_API
#endif
#else
// non-Windows: this is a NOP
#define HIAR_API
#endif

/*************************************************************************************************************
**************************************************************************************************************
    HIAR_CALL
*************************************************************************************************************/
#if defined( _WIN32 )
#define HIAR_CALL __stdcall
#else
// non-Windows: this is a NOP
#define HIAR_CALL
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
// public interface to face api


    
#define HIAR_FACE_FEATURE_LEN 512
#define HIAR_FACE_RECOG_MIN_SIZE 80
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Services provided by the SDK: interfaces

#ifdef __cplusplus
        extern "C"
        {
#endif

            /**
            * reocognizer initalized
            * @param sPath_Model    models path
            * @param sPath_Logger   logger file path, if sPath_Logger==nullptr, no log outputing.
            * @return HIAR_API int  result, 1:success, other:fail
            */
            HIAR_API int HiarFace_initRecognizer(const char* sPath_Model, const char* sPath_Logger);


            /**
            * extract faces features
            * @param ImgData    the character stream of  the image file including faces.
            * @param len_Img    length of the ImgData
            * @param face_bboxes   bound box of the face eg. {x_1,y_1,width_1,height_1,x_2,y_2,width_2,height_2...x_k,y_k,width_k,height_k},k = num_bbox
            * @param num_bbox    num of face bboxes
            * @param feature     extract features from faces,the length of each feature is HIAR_FACE_FEATURE_LEN or 512
            * @param len_features    *len_features is the length of feature, len_features = HIAR_FACE_FEATURE_LEN*num_bbox 
            * @return HIAR_API int  result, 1:success, other:fail
            */
            HIAR_API int HiarFace_extractFeature(const unsigned char* ImgData, const int len_Img, const int* face_bboxes, const int num_bbox, float **feature, int *len_features);

            
            /**
            * detect faces and extract faces features
            * @param ImgData    the character stream of  the image file including faces.
            * @param len_Img    length of the ImgData
            * @param face_bboxes   bound box of the face eg. {x_1,y_1,width_1,height_1,x_2,y_2,width_2,height_2...x_k,y_k,width_k,height_k},k = num_bbox
            * @param num_bbox    num of face bboxes
            * @param feature     extract features from faces,the length of each feature is HIAR_FACE_FEATURE_LEN or 512
            * @param len_features    the length of feature, len_features = HIAR_FACE_FEATURE_LEN*num_bbox
            * @return HIAR_API int  result, 1:success, other:fail
            */
            HIAR_API int HiarFace_detectAndExtractFeature(const unsigned char *ImgData, const int len_Img, int** face_bboxes, int *num_bbox, float** feature, int *len_features);

            /**
           * compare faces'features
           * @param faceA    the features of face A.
           * @param lenA    length of the faceA,is HIAR_FACE_FEATURE_LEN
           * @param faceB    the features of face B.
           * @param lenB    length of the faceB,is HIAR_FACE_FEATURE_LEN
           * @return HIAR_API float  result in [0,1], 0 -- face A and face B is different,1--face A and face B is same.
           */
            HIAR_API float HiarFace_compareFaceFeature(const float* faceA, const int lenA, const float* faceB, const int lenB);


            /**
           * compare two face images
           * @param ImgDataA    the character stream of  the image file including face A.
           * @param len_ImgA    length of the ImgDataA
           * @param face_bboxA   bound box of the face A eg. {x,y,width,height}
           * @param ImgDataB    the character stream of  the image file including face B.
           * @param len_ImgB    length of the ImgDataB
           * @param face_bboxB   bound box of the face B eg. {x,y,width,height}
           * @return HIAR_API float  result in [0,1], 0 -- face A and face B is different,1--face A and face B is same.
           */
            HIAR_API float HiarFace_compareFaceImage(const unsigned char *ImgDataA, const int len_ImgA, const int *face_bboxA, const unsigned char *ImgDataB, const int len_ImgB, const int* face_bboxB);


            /**
           * get qualities of face image crops
           * @param ImgData    the character stream of  the image file including faces.
           * @param len_Img    length of the ImgData
           * @param face_bboxes   bound box of the face eg. {x_1,y_1,width_1,height_1,x_2,y_2,width_2,height_2...x_k,y_k,width_k,height_k},k = num_bbox
           * @param num_bbox    num of face bboxes
           * @param face_quality     face crops's quality,its length is num_bbox,in[0,10],0--quality is the worst,10--quality is the best
           * @param face_direction   face crops's direction its length is num_bbox,score of direction > 0 is the front face ,or score of direction <=0 is side face
           * @return HIAR_API int  result, 1:success, other:fail
           */
            HIAR_API int HiarFace_getQualityFaceCrops(const unsigned char *ImgData, const int len_Img, const int* face_bbox, const int num_bbox, int* face_quality,float* face_direction);


            /**
            * release recognizer
            */
            HIAR_API void HiarFace_releaseRecognizer();

#ifdef __cplusplus
        }
#endif

