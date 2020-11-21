/********************************************************************
 * Copyright (c) 2020 John R. Patek
 * 
 * This software is provided 'as-is', without any express or implied 
 * warranty. In no event will the authors be held liable for any 
 * damages arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any 
 * purpose, including commercial applications, and to alter it and 
 * redistribute it freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you 
 *       must not claim that you wrote the original software. If you 
 *       use this software in a product, an acknowledgment in the 
 *       product documentation would be appreciated but is not 
 *       required.
 *    
 *    2. Altered source versions must be plainly marked as such, and 
 *       must not be misrepresented as being the original software.
 *    
 *    3. This notice may not be removed or altered from any source 
 *       distribution.
 * 
 *******************************************************************/
#include "record_manager.h"

static bool parse_args(
    int argc, 
    const char** argv, 
    uint16_t& port, 
    std::string& root_directory);

static void print_usage();

static void server_main(
    uint16_t port, 
    const std::string& root_directory);

int main(int argc, const char ** argv)
{
    uint16_t port;
    std::string root_directory;
    if(parse_args(argc,argv,port,root_directory))
    {
        server_main(
            port,
            root_directory);
    }
    else
    {
        print_usage();
    }
    return 0;
}

static bool parse_args(
    int argc, 
    const char** argv, 
    uint16_t& port, 
    std::string& root_directory)
{
    bool result(false);
    result = (argc == 3);
    if(result)
    {
        try
        {
            port = std::stoi(argv[1]);
        }
        catch(const std::exception& e)
        {
            result = false;
        }
    }
    root_directory = (result)?argv[2]:"";
    return result;
}

static void print_usage()
{
    std::cerr << "server <port> <root-directory>" << std::endl;
}

static void server_main(
    uint16_t port, 
    const std::string& root_directory)
{
    std::cerr << port           << std::endl
              << root_directory << std::endl;
}