#include "detector.hpp"

namespace rrec
{

cv::Mat Detector::get_image_main() { return image_main; }
cv::Mat Detector::get_image_L() { return image_L; }
cv::Mat Detector::get_image_d() { return image_d; }
cv::Mat Detector::get_image_clustered() { return image_clustered; }
void Detector::set_image_main(cv::Mat img) { img.copyTo(this->image_main); }

char Detector::pixel_from_intensity(std::vector<int> intensity, int num_pixels)
{
    // given an intensity vector and number of local pixels
}

void Detector::err_not_open()
{
    std::cout << "Error: couldn't open file" << std::endl;
}

void Detector::load_image()
{
    // load image at path this.path into image_main
    this->image_main = cv::imread(path);
    cv::cvtColor(this->image_main, this->image_main, CV_BGR2GRAY);

    if (image_main.empty())
    {
        is_open = false;
        int temp = 2;
        fwrite(&temp, 4, 1, stdout);
        fflush(stdout);
        std::cout << "image_main empty after call to imread" << std::endl;
    }
    else
    {
        is_open = true;
    }
}

void Detector::load_image(std::string path)
{
    // load image at path this.path into image_main

    this->path = path;
    load_image();
}

void Detector::load_pic(int rows, int cols)
{
    // load .pic file at this.path into image_main, at this point we know that
    // the pic and cutoff have been specified

    // try to open to .pic file
    std::ifstream inf(path, std::ios::binary);

    if (!inf.is_open())
    {
        // if we couldn't open, make it clear that it didn't work
        is_open = false;
        return;
    }

    // skip the first 624 bytes of the .pic file
    inf.ignore(624);

    // store the image temporarily in a vector
    std::vector<float> F(rows * cols);

    for (int i = 0; i < rows * cols; ++i)
    {
        float f;
        inf.read(reinterpret_cast<char *>(&f), sizeof(float));

        // make sure f is in [0, 256)
        if (f > pic_cutoff)
            f = 255;
        else
        {
            f *= 255.0f / pic_cutoff;
            F[i] = f;
        }
    }

    // convert image_main temporarily to 32bit floating point image type
    image_main.create(rows, cols, CV_32FC1); // and make sure it's the right size

    // memcpy the big floaty vector directly into image_main
    memcpy(image_main.data, F.data(), F.size() * sizeof(float));

    // convert the 4byte floaty image to an ordinary grayscale image
    image_main.convertTo(image_main, CV_8UC1);

    if (!image_main.empty())
    {
        is_open = true;
    }
}

void Detector::load_pic(float cutoff, int rows, int cols)
{
    pic_cutoff = cutoff;
    load_pic(rows, cols);
}

void Detector::load_pic(std::string path, int rows, int cols)
{
    // load .pic file at this.path into image_main
    this->path = path;
    load_pic(rows, cols);
}

void Detector::load_pic(std::string path, float cutoff, int rows, int cols)
{
    pic_cutoff = cutoff;
    this->path = path;
    load_pic(rows, cols);
}

Detector::Detector(std::string path) : path{path}, pic_cutoff{900}
{
    // this constructor should only be called to open an ordinary image
    if (path.substr(path.length() - 4, 4) == ".pic")
    {
        // we dont have enough information to open a .pic, so dont try
        is_open = false;
    }
    else
    {
        load_image();
    }
}

Detector::Detector(std::string path, int rows, int cols) : path{path},
                                                           pic_cutoff{900}
{
    // check if user wants to open a .pic or normal image file
    if (path.substr(path.length() - 4, 4) == ".pic")
    {
        load_pic(rows, cols);
    }
    else
    {
        load_image();
    }
}

Detector::Detector() : pic_cutoff{900} {} // only init pic cutoff value

void Detector::equalize()
{
    cv::equalizeHist(image_main, image_main);
}

void Detector::adaptive_hist_eq(int length)
{
    cv::Mat in_img = this->image_main;
    cv::Mat out_img;
    // first make sure that the outgoing image has the correct shape/type
    int rows = in_img.rows;
    int cols = in_img.cols;
    out_img.create(rows, cols, CV_8UC1);

    // grab some pointy bois
    unsigned char *in_pointer;
    unsigned char *out_pointer;

    // loop over all the pixels in the out_img
    for (int i = 0; i < rows; ++i)
    {
        out_pointer = out_img.ptr<unsigned char>(i);

        // create an empty std::vector to store pixel intensities
        std::vector<int> intensities(256, 0);

        // work out which rows in in_img are close to our row in out_img
        int row_beg = i - static_cast<int>(length / 2);
        int row_end = i + static_cast<int>(length / 2);
        // make sure there are no index-related segfaults!!
        if (row_beg < 0)
            row_beg = 0;
        if (row_end > rows - 1)
            row_end = rows - 1;

        // populate the intensities vector for the rectangular sub-image on the
        // new row (knowing we're necessarily at column 0)
        for (int a = row_beg; a <= row_end; ++a)
        {
            // get the pointer to this row in in_img
            in_pointer = in_img.ptr<unsigned char>(i);

            // this next loop is over cols near to column 0
            for (int b = 0; b <= length / 2; ++b)
            {
                // incrament element of intensities corresponding to pixel value
                ++(intensities[static_cast<int>(in_pointer[b])]);
            }
        }

        // now work out what the pixel at (i, 0) should be in out_image
        // first calculate the number of pixels used to work out intensity
        // int num_pixels = (length / 2 + 1) * (row_end - row_beg + 1);
        int num_pixels{0};
        for (auto t : intensities)
            num_pixels += t;

        // get the intensity of pixel (i, j) in in_img
        char init_intensity = in_img.ptr<unsigned char>(i)[0];

        // sum all pixels with intensity < init_intensity
        int sum{0};
        for (int temp = 0; temp < static_cast<int>(init_intensity); ++temp)
        {
            sum += intensities[temp];
        }

        // finally, to convert to proper units, divide by num_pixels
        out_pointer[0] = (255 * sum) / num_pixels;

        // now move on to cols which aren't the 0th col!
        bool last_col = false;         // check if we've done the last column
        for (int j = 1; j < cols; ++j) // j = 1(!!) as we already did column 0
        {
            // now work out which cols are 'near' to the j'th col
            int col_beg = j - static_cast<int>(length / 2);
            int col_end = j + static_cast<int>(length / 2);

            // bounds checking
            if (col_beg < 0)
                col_beg = 0;
            if (col_end > cols - 1)
                col_end = cols - 1;

            // if col_beg is still 0, dont subtract off intensities in col_beg-1
            if (col_beg != 0)
            {
                for (int a = row_beg; a <= row_end; ++a)
                {
                    in_pointer = in_img.ptr<unsigned char>(a);
                    // decrament element of intensities as this col is now
                    // "far away" from out current col
                    --(intensities[static_cast<int>(in_pointer[col_beg - 1])]);
                }
            }
            // if we've already done the last column, dont do it again
            if (!last_col)
            {
                // if we're doing the last col now, set last_col=true
                if (col_end == cols - 1)
                {
                    last_col = true;
                }

                for (int a = row_beg; a <= row_end; ++a)
                {
                    in_pointer = in_img.ptr<unsigned char>(a);
                    // this column is now "close" to our pixel in out_img
                    ++(intensities[static_cast<int>(in_pointer[col_end])]);
                }
            }

            // now work out what the pixel at (i, j) should be in out_image
            // first calculate the number of pixels used to work out intensity
            int num_pixels = (col_end - col_beg + 1) * (row_end - row_beg + 1);

            // get the intensity of pixel (i, j) in in_img
            char init_intensity = in_img.ptr<unsigned char>(i)[0];

            // sum all pixels with intensity < init_intensity
            int sum{0};
            for (int temp = 0; temp < static_cast<int>(init_intensity); ++temp)
            {
                sum += intensities[temp];
            }

            // finally, to convert to proper units, divide by num_pixels
            out_pointer[j] = (255 * sum) / num_pixels;
        }
    }
    out_img.copyTo(this->image_main);
}

void Detector::calculate_background(int L)
{
    cv::GaussianBlur(image_main, this->image_L, cv::Size(L, L), 0);
    if (!image_L.empty())
    {
        this->is_background = true;
    }
    else
    {
        this->is_background = false;
    }
}

void Detector::calculate_signal(int d)
{
    cv::GaussianBlur(image_main, this->image_d, cv::Size(d, d), 0);

    if (!image_L.empty())
    {
        this->is_signal = true;
    }
    else
    {
        this->is_signal = false;
    }
}

void Detector::calculate_significance(double sigma)
{

    int rows = image_main.rows;
    int cols = image_main.cols;

    // this makes sure that image_clustered is configured correctly
    image_main.copyTo(image_clustered);

    // if all images are stored continuously in memory then we need only grab a
    // uchar* pointer once per image
    if (image_d.isContinuous() && image_L.isContinuous() &&
        image_clustered.isContinuous())
    {
        cols *= rows;
        rows = 1;
    }

    // 256 channel colour <=> unsigned char
    unsigned char *imgPointer;
    unsigned char *brightnessPointer;
    unsigned char *thresholdPointer;

    // the mean of the uniform distribution the original picture was eq'd to
    double mu = 127.5;

    for (int i = 0; i < rows; ++i)
    {
        imgPointer = image_d.ptr<unsigned char>(i);
        brightnessPointer = image_L.ptr<unsigned char>(i);
        thresholdPointer = image_clustered.ptr<unsigned char>(i);

        for (int j = 0; j < cols; ++j)
        {
            double difference;
            double stdDev;
            if (brightnessPointer[j] > mu)
            {
                difference = brightnessPointer[j] - mu;
            }
            else
            {
                difference = -brightnessPointer[j] + mu;
            }

            // the standard deviation of the colours in the entire image is
            // 73.9. If brightnessPointer[j] != 127.5, then we are sampling
            // from some distribution which has been locally shifted. Whatever
            // distribution it is, we know that if brightnessPointer[j] = 255||0
            // then D = 127.5 and the standard deviation of the local
            // distribution is 0, as all pixels are the same and the
            // distribution is a delta function. For simplicity I linearly
            // interpolate the standard deviation for all other values, and set
            // the threshold value accordingly (although the exact stddev could
            // be calculated exactly for each pixel that would be far too
            // costly)
            stdDev = 73.9 * (1 - difference / 127.5) * sigma;

            if (imgPointer[j] > brightnessPointer[j] + stdDev / 2)
                thresholdPointer[j] = 255;
            else
                thresholdPointer[j] = 0;
        }
    }
}

void Detector::cluster()
{
    // use DBSCAN to cluster the significant pixels
    DBSCAN scanner{image_main.rows * image_main.cols};

    if (!image_clustered.empty())
    {
        this->clusters = scanner.getClusters(this->image_clustered,
                                             this->image_clustered);
    }
    else
    {
        this->clusters = scanner.getClusters(this->image_main,
                                             this->image_clustered);
    }
}

} // namespace rrec