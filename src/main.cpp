#include <cstdlib>
#include <exception>
#include <iostream>

#include "App.hpp"

using std::cout;
using std::endl;


int main(int, char* [])
{
    int result;
    try {
        App::initialize();
        result = App::run();
    }
    catch (std::exception& e) {
        cout << "ERROR: " << e.what() << endl;
        result = EXIT_FAILURE;
    }
    App::finalize();
    return result;
}
