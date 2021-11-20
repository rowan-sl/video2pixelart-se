#include "api.h++"

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

