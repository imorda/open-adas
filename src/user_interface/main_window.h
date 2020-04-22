#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QCloseEvent>
#include <QMessageBox>
#include <QShortcut>
#include <SDL.h>
#include <algorithm>
#include <thread>
#include <mutex>
#include <memory>
#include <opencv2/opencv.hpp>
#include <regex>
#include <sstream>
#include "file_storage.h"
#include "camera.h"
#include "lane_detector.h"
#include "object_detector.h"
#include "car_prop_reader.h"

#include "config.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showCam();
    void showCam(std::string video_path);
    void refreshCams();

protected:
    void closeEvent(QCloseEvent *event);
    void loadEffects();

private slots:
    void changeCam_clicked();
    
    
private:
    Ui::MainWindow *ui;

    // Current image
    // When user click "Capture", we take photo here then have it
    // to [Photos] folder
    cv::Mat current_img;
    std::mutex current_img_mutex;

    // File Storage
    ml_cam::FileStorage fs;

    QGraphicsPixmapItem pixmap;
    cv::VideoCapture video;

    // Camera to use
    std::vector<struct Camera> available_cams;
    int current_camera_index = 0;

    // Input from video (for demonstration)
    bool input_from_video = false;
    std::string video_path;

    std::shared_ptr<ObjectDetector> object_detector;
    std::shared_ptr<LaneDetector> lane_detector;
    std::shared_ptr<CarPropReader> car_prop_reader;

    std::vector<Detection> object_detection_results;
    std::mutex object_detection_results_mutex;

    cv::Mat lane_detection_results;
    std::mutex lane_detection_results_mutex;

public:
    void setCurrentImage(const cv::Mat & img);
    cv::Mat getCurrentImage();

    void setInputVideo(std::string video_path);


private:
    std::string getInputVideoPath();

    static void object_detection_thread(std::shared_ptr<ObjectDetector> object_detector, cv::Mat & img, std::mutex & img_mutex, 
    std::vector<Detection> & object_detection_results, std::mutex & object_detection_results_mutex);
    static void lane_detection_thread(std::shared_ptr<LaneDetector> lane_detector, cv::Mat & img, std::mutex & img_mutex, 
    cv::Mat & lane_detection_results, std::mutex & lane_detection_results_mutex);
    static void car_prop_reader_thread(std::shared_ptr<CarPropReader> car_prop_reader);
};

#endif // MAINWINDOW_H
