#include "record_manager.h"
#include <iostream>
int main(int argc, const char ** argv)
{
    std::cerr << rmp::djb_hash("johnpatek2@gmail.com") << std::endl;    
    return 0;
}