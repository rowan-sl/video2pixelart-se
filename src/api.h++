//describe stuff
#pragma once

#include <iostream>
#include <string>
#include <sys/ioctl.h>
//for argument parsing
#include <boost/program_options.hpp>
//opencv
#include <opencv2/opencv.hpp>
//progress bar
#include <cpptqdm/tqdm.h>

#include "settings.h++"

using namespace std;
using namespace cv;

typedef Point3_<uint8_t> Pixel;

struct ARGS;

template <class boost_err>
void handle_args_error (boost_err e);

void parse_args(int argc, char *argv[], ARGS &args);

std::string pxl2txt(int r, int g, int b);

std::string convert_frame(cv::Mat frame);

cv::Mat resize_image_to_term(cv::Mat input_image, struct winsize term_info);