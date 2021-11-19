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

bool NOCLEAR = false;

using namespace cv;
using namespace std;

typedef Point3_<uint8_t> Pixel;

const string block = "██";

struct ARGS
{
    string file_path;
    int cam_index;
    bool play_camera; // false = play from file, true = play from camera
    bool preprocess;
};

string pxl2txt(int r, int g, int b)
{
    string sequence = "\x1b[38;2;";
    sequence += to_string(r);
    sequence += ";";
    sequence += to_string(g);
    sequence += ";";
    sequence += to_string(b);
    sequence += "m";
    sequence += block;
    return sequence;
}

string convert_frame(Mat frame)
{
    string txt_img = "";

    for (int i = 0; i < frame.rows; i++)
    {
        for (int j = 0; j < frame.cols; j++)
        {
            Vec3b pxl = frame.at<cv::Vec3b>(i, j);
            txt_img += pxl2txt(pxl[2], pxl[1], pxl[0]);
        }
        txt_img += "\n";
    }

    return txt_img;
}

Mat resize_image_to_term(Mat input_image, struct winsize term_info)
{
    Mat scaled_img;

    int max_x = static_cast<int>(term_info.ws_col / 2); //because block is now 2 chars wide
    int max_y = term_info.ws_row;

    Size img_size = input_image.size();

    int orig_x = img_size.width;
    int orig_y = img_size.height;

    double scale_x = static_cast<double>(max_x) / static_cast<double>(orig_x);
    double scale_y = static_cast<double>(max_y) / static_cast<double>(orig_y);

    double scale;
    if (scale_x < scale_y)
    {
        scale = scale_x;
    }
    else
    {
        scale = scale_y;
    }
    cv::resize(input_image, scaled_img, Size(), scale, scale, INTER_LINEAR);
    return scaled_img;
}

template <class boost_err>
void handle_args_error (boost_err e) {
    clog << "error while parsing arguments:" << endl;
    clog << e.what() << endl;
    exit(1);
}

void parse_args(int argc, char *argv[], ARGS &args)
{
    try
    {
        namespace po = boost::program_options;
        //cli argument parsing
        po::options_description desc("Video2pixelart sea edition.\nConvert videos to pixelart and display them on the command line");

        desc.add_options()
        ("help,h", "produce help message")
        ("file,f", po::value<string>(), "video file for input")
        ("cam,c", po::value<int>(), "index of the camera to display from.")
        ("preprocess,p", po::bool_switch(), "process video before displaying it")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.contains("help"))
        {
            std::cout << desc << endl;
            std::exit(0);
            return;
        }
        if (!vm.contains("file") && !vm.contains("cam"))
        {
            //neither camera or file source specified
            std::cout << "must specify a input!\nuse --help for help" << endl;
            std::exit(1);
            return;
        }
        if (vm.contains("file") && vm.contains("cam"))
        {
            //bolth camera and file source specified
            std::cout << "you can only specify one source! (file/camera)" << endl;
            std::exit(1);
            return;
        }
        if (vm.contains("file"))
        {
            //file was specified
            args.file_path = vm["file"].as<string>();
            args.play_camera = false;
        }
        if (vm.contains("cam"))
        {
            //cam was specified
            args.cam_index = vm["cam"].as<int>();
            args.play_camera = true;
        }

        args.preprocess = vm["preprocess"].as<bool>();

        if (args.preprocess && args.play_camera)
        {
            //preprocessing with camera, which cant happen
            std::cout << "Cannot use preprocessing when reading from a camera!" << endl;
            std::exit(1);
        }
    } catch (boost::wrapexcept<boost::program_options::invalid_option_value> e) {
        handle_args_error<boost::wrapexcept<boost::program_options::invalid_option_value>>(e);
    } catch (boost::wrapexcept<boost::program_options::unknown_option> e) {
        handle_args_error<boost::wrapexcept<boost::program_options::unknown_option>>(e);
    }
}

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