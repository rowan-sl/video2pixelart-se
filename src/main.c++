#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <thread>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iterator>
#include <vector>
//for argument parsing
#include <boost/program_options.hpp>
//opencv
#include <opencv2/opencv.hpp>
//progress bar
#include <cpptqdm/tqdm.h>

#include "api.h++"
#include "conversion.c++"
#include "image_resize.c++"
#include "args.c++"

void preprocess_main(VideoCapture cap, struct winsize term_size, ARGS args, chrono::milliseconds frametime)
{
    //after all, why not unsigned long long
    long long unsigned int num_frames = cap.get(CAP_PROP_FRAME_COUNT);
    Mat image;
    vector<string> frames; //storing frames for preprocessing
    tqdm progbar;

    std::cout << "processing frames..." << endl;

    long long unsigned int current_frame = 0;

    while (true)
    {
        cap >> image;
        if (image.empty())
        {
            break;
        }

        Mat scaled_img = resize_image_to_term(image, term_size);

        string txt_img = convert_frame(scaled_img);

        progbar.progress(current_frame, num_frames);
        frames.push_back(txt_img);
        current_frame += 1;
    }

    progbar.finish();

    for (string frame : frames)
    {
        if (!NOCLEAR)
        {
            std::cout << "\x1b[2J" << endl;
        }                      //clear screen
        std::cout << frame << endl; //display image
        this_thread::sleep_for(frametime);
    }
}

void normal_main(VideoCapture cap, struct winsize term_size, ARGS args, chrono::milliseconds frametime)
{
    Mat image;

    while (true)
    {
        cap >> image;
        if (image.empty())
        {
            break;
        }

        Mat scaled_img = resize_image_to_term(image, term_size);

        string txt_img = convert_frame(scaled_img);

        if (!NOCLEAR)
        {
            std::cout << "\x1b[2J" << endl;
        }                        //clear screen
        std::cout << txt_img << endl; //display image
        this_thread::sleep_for(frametime);
    }
}

int main(int argc, char *argv[])
{
    ARGS args;
    parse_args(argc, argv, args);

    struct winsize term_size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &term_size);

    Mat image;
    std::cout << "opening file/camera" << endl;
    VideoCapture cap;
    if (args.play_camera)
    {
        //camera
        cap = VideoCapture(args.cam_index);
    }
    else
    {
        //file
        cap = VideoCapture(args.file_path);
    }
    if (!cap.isOpened())
    {
        std::cout << "cannot open file/camera!" << endl;
        std::cout << "are you shure that you entered it correctly?" << endl;
        std::exit(1);
    }

    chrono::milliseconds frametime = chrono::milliseconds(static_cast<int>(cap.get(CAP_PROP_FPS)));

    if (args.preprocess) {
        preprocess_main(cap, term_size, args, frametime);
    } else {
        normal_main(cap, term_size, args, frametime);
    }

    if (!NOCLEAR)
    {
        std::cout << "\x1b[0m"
             << "\x1b[2J" << endl; //reset the graphics mode of the terminal
    }

    cap.release();
    return 0;
}