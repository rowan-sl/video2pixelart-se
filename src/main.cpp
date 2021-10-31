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

#define printm(x) cout << x << endl;

using namespace cv;
using namespace std;

namespace po = boost::program_options;

typedef Point3_<uint8_t> Pixel;

const string block = "██";

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

int main(int argc, char *argv[])
{
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

    int source_cam;
    string source_file;
    int source_type;//1 for cam, 0 for file
    bool preprocess = false;

    if (vm.count("help"))
    {
        cout << desc << endl;
        return 0;
    }
    if (!vm.count("file") && !vm.count("cam"))
    {
        //neither camera or file source specified
        cout << "must specify a input!\nuse --help for help" << endl;
        return 1;
    }
    if (vm.count("file") && vm.count("cam"))
    {
        //bolth camera and file source specified
        cout << "you can only specify one source! (file/camera)" << endl;
        return 1;
    }
    if (vm.count("file"))
    {
        //file was specified
        try {
            source_file = vm["file"].as<string>();
            source_type = 0;
        } catch(boost::bad_any_cast) {
            cout << "Bad file argument!" << endl;
            return 1;
        }
        source_type = 0;
    }
    if (vm.count("cam")) {
        //cam was specified
        try {
            source_cam = vm["cam"].as<int>();
            source_type = 1;
        } catch(boost::bad_any_cast) {
            cout << "Bad file argument!" << endl;
            return 1;
        }
    }
    if (vm["preprocess"].as<bool>()) {
        //preprocessing
        preprocess = true;
    }

    if (preprocess && source_type) {
        //preprocessing with camera, which cant happen
        cout << "Cannot use preprocessing when reading from a camera!" << endl;
        return 1;
    }


    struct winsize term_size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &term_size);
    //cout << term_size.ws_col << "," << term_size.ws_row << endl;

    Mat image;

    VideoCapture cap;
    if (source_type == 1) {
        //camera
        cap = VideoCapture(source_cam);
    } else if (source_type == 0) {
        //file
        cap = VideoCapture(source_file);
    } else {
        cout << "unknown input type (this should never happen)" << endl;
        return 42;
    }

    chrono::milliseconds frametime = chrono::milliseconds(static_cast<int>(cap.get(CAP_PROP_FPS)));
    //after all, why not unsigned long long
    unsigned long long int num_frames = cap.get(CAP_PROP_FRAME_COUNT);

    if (!cap.isOpened())
    {

        cout << "cannot open camera";
    }

    vector<string> frames;//storing frames for preprocessing
    tqdm progbar;
    if (preprocess) {
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
        //resize(image, scaled_img, Size(term_size.ws_col, term_size.ws_row), INTER_LINEAR);
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
                // You can now access the pixel value with cv::Vec3b
                Vec3b pxl = scaled_img.at<cv::Vec3b>(i, j);
                txt_img += pxl2txt(pxl[2], pxl[1], pxl[0]);
            }
            txt_img += "\n";
        }
        if (!preprocess) {
            cout << "\x1b[2J" << endl; //clear screen
            cout << txt_img << endl;   //display image
            this_thread::sleep_for(frametime);
        } else {
            progbar.progress(current_frame, num_frames);
            frames.push_back(txt_img);
            current_frame += 1;
        }
    }

    progbar.finish();

    if (preprocess) {
        for (string frame: frames) {
            cout << "\x1b[2J" << endl; //clear screen
            cout << frame << endl;   //display image
            this_thread::sleep_for(frametime);
        }
    }

    cout << "\x1b[0m"
         << "\x1b[2J" << endl; //reset the graphics mode of the terminal
    cap.release();
    return 0;
}