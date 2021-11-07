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

struct ARGS {
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

void parse_args(int argc, char *argv[], ARGS &args) {
    try {
        namespace po = boost::program_options;
        //cli argument parsing
        po::options_description desc("Video2pixelart sea edition.\nConvert videos to pixelart and display them on the command line");
        desc.add_options()
            ("help", "produce help message")
            ("file,f", po::value<string>(), "video file for input")
            ("cam,c", po::value<int>(), "index of the camera to display from.")
            ("preprocess,p", po::bool_switch(), "process video before displaying it")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.contains("help"))
        {
            cout << desc << endl;
            exit(0);
            return;
        }
        if (!vm.contains("file") && !vm.contains("cam"))
        {
            //neither camera or file source specified
            cout << "must specify a input!\nuse --help for help" << endl;
            exit(1);
            return;
        }
        if (vm.contains("file") && vm.contains("cam"))
        {
            //bolth camera and file source specified
            cout << "you can only specify one source! (file/camera)" << endl;
            exit(1);
            return;
        }
        if (vm.contains("file"))
        {
            //file was specified
            args.file_path = vm["file"].as<string>();
            args.play_camera = false;
        }
        if (vm.contains("cam")) {
            //cam was specified
            args.cam_index = vm["cam"].as<int>();
            args.play_camera = true;
        }

        args.preprocess = vm["preprocess"].as<bool>();

        if (args.preprocess && args.play_camera) {
            //preprocessing with camera, which cant happen
            cout << "Cannot use preprocessing when reading from a camera!" << endl;
            exit(1);
        }
    } catch (boost::wrapexcept<boost::program_options::invalid_option_value> e) {
        clog << "error while parsing arguments:" << endl;
        clog << e.what() << endl;
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    ARGS args;
    parse_args(argc, argv, args);

    struct winsize term_size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &term_size);
    //cout << term_size.ws_col << "," << term_size.ws_row << endl;

    Mat image;
    cout << "opening file/camera" << endl;
    VideoCapture cap;
    if (args.play_camera) {
        //camera
        cap = VideoCapture(args.cam_index);
    } else {
        //file
        cap = VideoCapture(args.file_path);
    }

    chrono::milliseconds frametime = chrono::milliseconds(static_cast<int>(cap.get(CAP_PROP_FPS)));
    //after all, why not unsigned long long
    unsigned long long int num_frames = cap.get(CAP_PROP_FRAME_COUNT);
    if (!cap.isOpened())
    {
        cout << "cannot open file/camera!" << endl;
        cout << "are you shure that you entered it correctly?" << endl;
        exit(1);
    }

    vector<string> frames;//storing frames for preprocessing
    tqdm progbar;
    if (args.preprocess) {
        cout << "processing frames..." << endl;
    }

    int current_frame = 0;

    while (true)
    {
        cap >> image;
        if (image.empty())
        {
            break;
        }
        Mat scaled_img;
        int max_x = static_cast<int>(term_size.ws_col/2);//because block is now 2 chars wide
        int max_y = term_size.ws_row;
        Size img_size = image.size();
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
        cv::resize(image, scaled_img, Size(), scale, scale, INTER_LINEAR);

        string txt_img = "";

        for (int i = 0; i < scaled_img.rows; i++)
        {
            for (int j = 0; j < scaled_img.cols; j++)
            {
                Vec3b pxl = scaled_img.at<cv::Vec3b>(i, j);
                txt_img += pxl2txt(pxl[2], pxl[1], pxl[0]);
            }
            txt_img += "\n";
        }
        if (!args.preprocess) {
            if (!NOCLEAR) {cout << "\x1b[2J" << endl;} //clear screen
            cout << txt_img << endl;   //display image
            this_thread::sleep_for(frametime);
        } else {
            progbar.progress(current_frame, num_frames);
            frames.push_back(txt_img);
            current_frame += 1;
        }
    }

    progbar.finish();

    if (args.preprocess) {
        for (string frame: frames) {
            if (!NOCLEAR) {cout << "\x1b[2J" << endl;} //clear screen
            cout << frame << endl;   //display image
            this_thread::sleep_for(frametime);
        }
    }

    if (!NOCLEAR) {
        cout << "\x1b[0m" << "\x1b[2J" << endl; //reset the graphics mode of the terminal
    }

    cap.release();
    return 0;
}