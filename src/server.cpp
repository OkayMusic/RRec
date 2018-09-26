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
    // send an error response
    int temp = static_cast<int>(response_type::error);
    fwrite(&temp, 4, 1, stdout);
    fflush(stdout);
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
        int temp = static_cast<int>(response_type::success);
        fwrite(&temp, 4, 1, stdout);
        fflush(stdout);

        cv::Mat image(detector.get_image_clustered());

        // write the data in one shot
        fwrite(image.data, 1, image.rows * image.cols, stdout);
        fflush(stdout);
    }
}

void Server::handle_NotImplemented()
{
    int temp = static_cast<int>(response_type::not_implemented);
    fwrite(&temp, 4, 1, stdout);
    fflush(stdout);
}

void Server::listen_to_python(int mode)
{
    if (mode == static_cast<int>(server_type::offline))
    {
        while (1 < 2)
        {
            unsigned int instruction;
            fread(&instruction, 4, 1, stdin);

            // int temp = static_cast<int>()

            // if there are any arguments which we need to grab from the python end,
            // grab them here and then pass them to the appropriate handler.
            // if we don't need to grab any args from stdin, then just call the
            // relevant handler right away.
            switch (instruction)
            {
            case imageRequest:
            {
                handle_ImageRequest();
                break;
            }
            case loadFromFile:
            {
                // grab the path to the file from stdin
                std::string path;
                std::getline(std::cin, path);

                handle_LoadFromFile(path);

                // if execution reached here, return success
                int temp = static_cast<int>(response_type::success);
                fwrite(&temp, 4, 1, stdout);
                fflush(stdout);
                break;
            }
            case loadFromPython:
            {
                // NOT IMPLEMENTED
                handle_NotImplemented();
                break;
            }
            case runAlgorithm:
            { // NOT IMPLEMENTED
                handle_NotImplemented();
                break;
            }
            case equalize:
            {
                // simply call the equalize method, more error checks needed
                handle_Equalize();

                // if execution reached here, return success
                int temp = static_cast<int>(response_type::success);
                fwrite(&temp, 4, 1, stdout);
                fflush(stdout);
                break;
            }
            case calculateBackground:
            {
                // grab L parameter
                int L;
                fread(&L, 4, 1, stdin);
                handle_CalculateBackground(L);

                // if execution reached here, return success
                int temp = static_cast<int>(response_type::success);
                fwrite(&temp, 4, 1, stdout);
                fflush(stdout);
                break;
            }
            case calculateSignal:
            {
                // int temp = static_cast<int>(response_type::error);
                // fwrite(&temp, 4, 1, stdout);
                // std::cout << "testing" << std::endl;
                // fflush(stdout);
                int d;
                fread(&d, 4, 1, stdin);

                handle_CalculateSignal(d);
                // if execution reached here, return success
                int temp = static_cast<int>(response_type::success);
                fwrite(&temp, 4, 1, stdout);
                fflush(stdout);
                break;
            }
            case calculateSignificance:
            {
                double sigma;

                fread(&sigma, sizeof(double), 1, stdin);
                handle_CalculateSignificance(sigma);

                // if execution reached here, return success
                int temp = static_cast<int>(response_type::success);
                fwrite(&temp, 4, 1, stdout);
                fflush(stdout);
                break;
            }
            case cluster:
            {
                handle_Cluster();

                // if execution reached here, return success
                int temp = static_cast<int>(response_type::success);
                fwrite(&temp, 4, 1, stdout);
                fflush(stdout);
                break;
            }

            default:
                // if execution reaches here, request isn't implemented
                int temp = static_cast<int>(response_type::not_implemented);
                fwrite(&temp, 4, 1, stdout);
                fflush(stdout);
                std::cout << instruction << std::endl;
                break;
            }
        }
    }
    else
    {
        handle_NotImplemented();
    }
}

} // namespace rrec