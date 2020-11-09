#include "record_manager.h"

int main(int argc, const char ** argv)
{
    std::fstream file(rmp::djb_hash("johnpatek2@gmail.com"),std::ios::out);
    file << "hello" << std::endl;
    return 0;
}