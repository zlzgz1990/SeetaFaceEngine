#ifndef ANDROID
#include <iostream>

#include "cv.h"
#include "highgui.h"

#include "face_detection.h"
#include "face_alignment.h"

#define TEST_WINDOW_NAME "FaceAlignTest"

#ifdef __GNUC__
#include <sys/time.h>
#define timeOfFunc(Func, args...) ({ \
    long ret = 0L; \
    struct timeval tvs, tve; \
    gettimeofday(&tvs, NULL); \
    Func(args);\
    gettimeofday(&tve, NULL); \
    ret = (tve.tv_sec - tvs.tv_sec) * 1000 + (tve.tv_usec - tvs.tv_usec) / 1000; \
})
#define timeOfOutFunc(out, Func, args...) ({ \
    long ret = 0L; \
    struct timeval tvs, tve; \
    gettimeofday(&tvs, NULL); \
    out = Func(args);\
    gettimeofday(&tve, NULL); \
    ret = (tve.tv_sec - tvs.tv_sec) * 1000 + (tve.tv_usec - tvs.tv_usec) / 1000; \
})
#endif // __GNUC__

#ifdef _WIN32
#define SEETA_FD_MODEL_PATH "../../model/seeta_fd_frontal_v1.0.bin"
#define SEETA_FA_MODEL_PATH "../../model/seeta_fa_v1.1.bin"
#define FACES_IMAGE_PATH "../../data/faces.png"
#else
#define SEETA_FD_MODEL_PATH "model/seeta_fd_frontal_v1.0.bin"
#define SEETA_FA_MODEL_PATH "model/seeta_fa_v1.1.bin"
#define FACES_IMAGE_PATH "data/face.png"
#endif

int main(int argc, char** argv) {
    const char *fd_path = SEETA_FD_MODEL_PATH;
    const char *fa_path = SEETA_FA_MODEL_PATH;
    const char *image_path = FACES_IMAGE_PATH;
    if (argc >= 4) {
        fd_path = argv[1];
        fa_path = argv[2];
        image_path = argv[3];
    }

    //load image
    IplImage *img_grayscale = NULL;
    img_grayscale = cvLoadImage(image_path, 0);
    if (!img_grayscale) {
        return 0;
    }

    IplImage *img_color = cvLoadImage(image_path, 1);

    // Initialize face detection model
    seeta::FaceDetection detector(fd_path);
    detector.SetMinFaceSize(40);
    detector.SetScoreThresh(2.f);
    detector.SetImagePyramidScaleFactor(0.2f);
    detector.SetWindowStep(4, 4);

    // Initialize face alignment model
    seeta::FaceAlignment point_detector(fa_path);

    int pts_num = 5;
    int im_width = img_grayscale->width;
    int im_height = img_grayscale->height;
    unsigned char* data = new unsigned char[im_width * im_height];
    unsigned char* data_ptr = data;
    unsigned char* image_data_ptr = (unsigned char*)img_grayscale->imageData;
    int h = 0;
    for (h = 0; h < im_height; h++) {
        memcpy(data_ptr, image_data_ptr, im_width);
        data_ptr += im_width;
        image_data_ptr += img_grayscale->widthStep;
    }

    seeta::ImageData image_data;
    image_data.data = data;
    image_data.width = im_width;
    image_data.height = im_height;
    image_data.num_channels = 1;

    // Detect faces
#ifdef __GNUC__
    std::vector<seeta::FaceInfo> faces;
    long time_fd = timeOfOutFunc(faces, detector.Detect, image_data), time_fa;
#else
    std::vector<seeta::FaceInfo> faces = detector.Detect(image_data);
#endif // __GNUC__
    size_t face_num = faces.size();
    if (face_num == 0) {
        delete[]data;
        cvReleaseImage(&img_grayscale);
        cvReleaseImage(&img_color);
        return 0;
    }

    for(size_t face_index = 0; face_index < face_num; face_index++) {
        // Detect 5 facial landmarks
        seeta::FacialLandmark points[5];
#ifdef __GNUC__
        time_fa += timeOfFunc(point_detector.PointDetectLandmarks, image_data, faces[face_index], points);
#else
        point_detector.PointDetectLandmarks(image_data, faces[face_index], points);
#endif // __GNUC__
        // Visualize the results
        cvRectangle(img_color,
            cvPoint(faces[face_index].bbox.x, faces[face_index].bbox.y),
            cvPoint(faces[face_index].bbox.x + faces[face_index].bbox.width - 1,
                    faces[face_index].bbox.y + faces[face_index].bbox.height - 1), CV_RGB(255, 0, 0));

        for (int i = 0; i<pts_num; i++) {
            cvCircle(img_color, cvPoint(points[i].x, points[i].y), 2, CV_RGB(0, 255, 0), CV_FILLED);
        }
    }

#ifdef __GNUC__
    std::cout << "FaceDetection TimeCost=" << time_fd << "\tFaceAlignment TimeCost=" << time_fa << std::endl;
#endif // __GNUC__

    // cvSaveImage("result.jpg", img_color);

    cvNamedWindow(TEST_WINDOW_NAME, CV_WINDOW_AUTOSIZE);
    cvShowImage(TEST_WINDOW_NAME, img_color);
    cvWaitKey(0);
    cvDestroyAllWindows();

    // Release memory
    cvReleaseImage(&img_color);
    cvReleaseImage(&img_grayscale);
    delete[]data;
    return 0;
}
#endif // ANDROID
