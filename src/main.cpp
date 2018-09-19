#include <iostream>
#include <fstream>
#include <string>

#include "screencap.hpp"
#include "processing.hpp"
#include "clustering.hpp"
#include "LTEM.hpp"

int main()
{
    // std::string path = "../video/LTEM_cam.mpg";
    // std::cout << path << std::endl;
    // rrec::detectVideo(path);

    handleSamLTEMAnalysis();

    return 0;
}