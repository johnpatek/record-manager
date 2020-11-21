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

static bool client_init(
    std::shared_ptr<rmp::client>& client,
    int argc, 
    const char ** argv) noexcept;

static bool client_main(
    std::shared_ptr<rmp::client>& client) noexcept;

int main(int argc, const char ** argv)
{
    std::shared_ptr<rmp::client> client;
    return (client_init(client,argc,argv) 
        && client_main(client)) 
        ? EXIT_SUCCESS:EXIT_FAILURE;
}

static bool client_init(
    std::shared_ptr<rmp::client>& client,
    int argc, 
    const char ** argv) noexcept
{
    bool result(true);
    std::string error_message;
    std::string remote_host;
    uint16_t remote_port;

    result = (argc == 3);

    if(result)
    {
        try
        {
            remote_host = argv[1];
            remote_port = std::stoi(argv[2]);
            client = std::make_shared<rmp::client>(
                remote_host,
                remote_port);
        }
        catch(const std::exception& e)
        {
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
                  << "client <address> <port>"
                  << std::endl;
    }

    return result;
}

static bool client_main(
    std::shared_ptr<rmp::client>& client) noexcept
{
    bool result(true);
    bool loop(true);
    while(result && loop)
    {
        loop = false;
    }
    return result;
}