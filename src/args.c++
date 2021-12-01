#include "api.h++"
struct ARGS
{
    string file_path;
    int cam_index;
    bool play_camera; // false = play from file, true = play from camera
    bool preprocess;
    bool nodisplay;
    bool image;//just do one image, not a video
    bool JUST_DO_IT; // screw the consiquences. disables stuff
};

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
        ("img,i", po::value<string>(), "pixelart a image, not a whole video")
        ("preprocess,p", po::bool_switch(), "process video before displaying it")
        ("nodisplay,n", po::bool_switch(), "do not display the processed video. usefull for speed benchmarks")
        ("do-it-now", po::bool_switch(), "do it. i dont care what happens, JUST DO IT!!!")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.contains("do-it-now")) {
            args.JUST_DO_IT = true;
        }

        if (vm.contains("help"))
        {
            std::cout << desc << endl;
            std::exit(0);
            return;
        }
        bool dofile = vm.contains("file");
        bool docam = vm.contains("cam");
        bool doimg = vm.contains("img");
        if (!dofile && !docam && !doimg)
        {
            //neither camera or file source specified
            std::cout << "must specify a input!\nuse --help for help" << endl;
            std::exit(1);
            return;
        }
        if (dofile && docam || dofile && doimg || docam && doimg)
        {
            //more than 1 source
            std::cout << "you can only specify one source! (file/camera/image)" << endl;
            std::exit(1);
            return;
        }
        if (vm.contains("file"))
        {
            //file was specified
            args.file_path = vm["file"].as<string>();
            args.play_camera = false;
        }
        if (vm.contains("img"))
        {
            //img was specified
            args.file_path = vm["img"].as<string>();
            args.image = true;
            args.play_camera = false;
        }
        if (vm.contains("cam"))
        {
            //cam was specified
            args.cam_index = vm["cam"].as<int>();
            args.play_camera = true;
        }

        args.preprocess = vm["preprocess"].as<bool>();
        args.nodisplay = vm["nodisplay"].as<bool>();

        if (args.preprocess && args.play_camera)
        {
            //preprocessing with camera, which cant happen
            std::cout << "Cannot use preprocessing when reading from a camera!" << endl;
            std::cout << "like actualy its impossible" << endl;
            std::exit(1);
        }
        if (args.nodisplay && args.play_camera) {
            if (args.JUST_DO_IT) {
                std::cout << "i guess if you realy want to..." << endl;
            } else {
                std::cout << "what are you even trying to do....?" << endl;
                std::cout << "there is literaly no point in this..." << endl;
                std::cout << "...use --do-it-now to override this" << endl;
                std::exit(1);
            }
        }
        if (!args.preprocess && args.nodisplay) {
            if (args.JUST_DO_IT) {
                std::cout << "i guess if you realy want to..." << endl;
            } else {
            std::cout << "srsly?" << endl;
            std::cout << "there is literaly no point in this..." << endl;
            std::cout << "...use --do-it-now to override this" << endl;
            std::exit(-1);
            }
        }
    } catch (boost::wrapexcept<boost::program_options::invalid_option_value> e) {
        handle_args_error<boost::wrapexcept<boost::program_options::invalid_option_value>>(e);
    } catch (boost::wrapexcept<boost::program_options::unknown_option> e) {
        handle_args_error<boost::wrapexcept<boost::program_options::unknown_option>>(e);
    }
}