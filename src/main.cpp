#include <exception>
#include <iostream>

#include "App.hpp"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

using std::cout;
using std::cerr;
using std::endl;


int main(int, char* [])
{
    try {
        App app;
        return app.run();
    }
    catch (std::exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }
}
