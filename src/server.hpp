#pragma once

#include <vector>

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

  public:
    Server();

    // all of the handlers implemented by the server
    void handle_BadInput();
    void handle_ImageRequest();
    void handle_LoadFromFile();
    void handle_LoadFromPython();
    void handle_RunAlgorithm();
    void handle_Equalize();
    void handle_CalculateBackground();
    void handle_CalculateSignal();
    void handle_CalculateSignificance();
    void handle_Cluster();

    // listens to stdin and calls an appropriate handler depending on input
    void listen_to_python();
};
} // namespace rrec