#include "api.h++"

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