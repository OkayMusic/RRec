#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "detector.hpp"

namespace rrec
{
class Server
{
  private:
    Detector detector;

    // these enums dictate the content of the incoming python request
    enum message_type
    {
        imageRequest,
        loadFromFile,
        loadFromPython,
        runAlgorithm,
        equalize,
        calculateBackground,
        calculateSignal,
        calculateSignificance,
        cluster
    };

    enum class response_type
    {
        success,         // returned on success, the default response type
        not_implemented, // returned on call to not implemented method
        error            // other undefined error
    };

    enum image_type
    {
        image_main,
        image_clustered,
        image_L,
        image_d
    };

    enum class server_type
    {
        online,
        offline
    };

  public:
    Server();
    Server(std::string path);
    Server(std::string path, int rows, int cols);

    // all of the handlers implemented by the server
    void handle_BadInput(std::string err_msg);
    void handle_ImageRequest();
    void handle_LoadFromFile(std::string path);
    void handle_LoadFromFile(std::string path, int rows, int cols);
    void handle_LoadFromPython();
    void handle_Equalize();
    void handle_CalculateBackground(int L);
    void handle_CalculateSignal(int d);
    void handle_CalculateSignificance(double sigma);
    void handle_Cluster();
    void handle_FourierPrep();    // writes in smtest file format
    void handle_NotImplemented(); // called in place of NI methods
    void handle_Success();        // called after success

    // listens to stdin and calls an appropriate handler depending on input
    void listen_to_python(int mode);
};
} // namespace rrec