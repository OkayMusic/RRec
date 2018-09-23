#include "server.hpp"

namespace rrec
{
Server::Server() {} // default constructor doesn't currently do anything
// the other constructors just instantiate a detector
Server::Server(std::string path) : detector{path}
{
    if (!detector.is_open)
    {
        detector.err_not_open();
    }
}

Server::Server(std::string path, int rows, int cols)
    : detector{path, rows, cols}
{
    if (!detector.is_open)
    {
        detector.err_not_open();
    }
}

void Server::handle_BadInput(std::string err_msg)
{
    // NOT IMPLEMENTED
    std::cout << err_msg << std::endl;
}

void Server::handle_LoadFromFile(std::string path)
{
    // work out if user wants to load a stored .pic file or ordinary image file
    if (path.substr(path.length() - 4, 4) == ".pic")
    {
        // we can't open .pic files without knowing how many rows/cols there are
        detector.is_open = false;
        handle_BadInput("to open a .pic file pass N_rows and N_cols as args.");
    }
    else
    {
        detector.load_image(path);
    }
}

void Server::handle_LoadFromFile(std::string path, int rows, int cols)
{
    // work out if user wants to load a stored .pic file or ordinary image file
    if (path.substr(path.length() - 4, 4) == ".pic")
    {
        // we have enough information to open a .pic file
        detector.load_pic(path, rows, cols);
    }
    else
    {
        detector.load_image(path);
    }
}

void Server::handle_Equalize()
{
    if (detector.is_open)
    {
        detector.equalize();
    }
    else
    {
        handle_BadInput("file not open.");
    }
}

void Server::handle_CalculateBackground(int L)
{
    if (detector.is_open)
    {
        detector.calculate_background(L);
    }
    else
    {
        handle_BadInput("file not open.");
    }
}

void Server::handle_CalculateSignal(int d)
{
    if (detector.is_open)
    {
        detector.calculate_signal(d);
    }
    else
    {
        handle_BadInput("file not open.");
    }
}

void Server::handle_CalculateSignificance(double sigma)
{
    if (!detector.is_open)
    {
        handle_BadInput("file not open.");
    }
    else if (!detector.is_background)
    {
        handle_BadInput("background not calculated.");
    }
    else if (!detector.is_signal)
    {
        handle_BadInput("signal not calculated.");
    }
    else
    {
        detector.calculate_significance(sigma);
    }
}

void Server::handle_Cluster()
{
    if (!detector.is_open)
    {
        handle_BadInput("file not open.");
    }
    detector.cluster();
}

void Server::handle_ImageRequest()
{
    // not implemented (fully)
    if (!detector.is_open)
    {
        handle_BadInput("file not open.");
        return;
    }
    else
    {
        cv::Mat image_main(detector.get_image_main());
        for (int i = 0; i < image_main.cols; ++i)
        {
            fwrite(image_main.data + i * image_main.rows, 1,
                   image_main.rows, stdout);
            std::cout << std::endl;
        }
    }
}

} // namespace rrec