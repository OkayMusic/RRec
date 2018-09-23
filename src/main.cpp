#include <iostream>
#include <fstream>
#include <string>

#include "server.hpp"

int main()
{
    const int rows{1296};
    const int cols{1728};

    rrec::Server main_server{};
    main_server.handle_LoadFromFile("_.pic", rows, cols);

    // main_server.handle_Equalize(); // eq main image
    // main_server.handle_CalculateBackground(51);
    // main_server.handle_CalculateSignal(9);
    // main_server.handle_CalculateSignificance(0.6);
    // main_server.handle_Cluster();
    main_server.handle_ImageRequest(); // print main_image to stdout

    // std::cout << "hello" << std::endl;

    return 0;
}