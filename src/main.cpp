#include <iostream>
#include <fstream>
#include <string>

#include "server.hpp"

int main()
{
    // const int rows{1296};
    // const int cols{1728};

    rrec::Server main_server{};
    main_server.listen_to_python(1);

    return 0;
}