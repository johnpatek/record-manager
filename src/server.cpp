/********************************************************************
 * Copyright (c) 2021 John R. Patek
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

static bool server_init(
    std::shared_ptr<rmp::server>& server,
    int argc, 
    const char ** argv);

static bool server_main(
    std::shared_ptr<rmp::server>& server) noexcept;

int main(int argc, const char ** argv)
{
    std::shared_ptr<rmp::server> server;
    return (server_init(server,argc,argv) 
        && server_main(server))
        ? EXIT_SUCCESS:EXIT_FAILURE;
}

static bool server_init(
    std::shared_ptr<rmp::server>& server,
    int argc, 
    const char ** argv)
{
    bool result(true);
    std::string error_message;
    uint16_t remote_port;
    std::string root_directory;

    result = (argc == 3);

    if(result)
    {
        try
        {
            remote_port = std::stoi(argv[1]);
            root_directory = argv[2];
            server = std::make_shared<rmp::server>(
                remote_port,
                root_directory);
        }
        catch(const std::exception& e)
        {
            error_message = e.what();
            result = false;
        }
    }
    else
    {
        error_message = "3 args expected, " + std::to_string(argc) + " found.";
    }
    

    if (!result)
    {
        std::cerr << "Failed to parse args: "
                  << error_message
                  << std::endl
                  << "server <port> <root directory>"
                  << std::endl;
    }

    return result;
}

#ifdef _WIN32

#else
static bool server_main(
    std::shared_ptr<rmp::server>& server) noexcept
{
    try
    {
        server->start();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return true;
}
#endif