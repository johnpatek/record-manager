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

static std::string get_input(
    const std::string& prompt);

static void create_command(
    std::shared_ptr<rmp::client>& client);

static void read_command(
    std::shared_ptr<rmp::client>& client);

static void update_command(
    std::shared_ptr<rmp::client>& client);

static void delete_command(
    std::shared_ptr<rmp::client>& client);

static void print_help();

static bool client_main(
    std::shared_ptr<rmp::client>& client) noexcept
{
    bool result(true);
    bool loop(true);
    std::string line_buffer;
    while(result && loop)
    {
        std::cerr << ">>> ";
        std::getline(std::cin,line_buffer);
        try
        {        
            if(line_buffer == "create")
            {
                create_command(client);
            }
            else if(line_buffer == "read")
            {
                read_command(client);
            }
            else if(line_buffer == "update")
            {
                update_command(client);
            }
            else if(line_buffer == "delete")
            {
                delete_command(client);
            }
            else if(line_buffer == "exit")
            {
                loop = false;
            }
            else if(line_buffer == "help")
            {
                print_help();
            }
            else
            {
                std::cerr << "Unrecognized command. " 
                        << "Enter \"help\" "
                        << "for more information." 
                        << std::endl;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            result = false;
        }
    }
    return result;
}

static std::string get_input(
    const std::string& prompt)
{
    std::string result;
    std::cerr << prompt << ": ";
    std::getline(std::cin,result);
    return result;
}

static void create_command(
    std::shared_ptr<rmp::client>& client)
{
    std::string email,name,phone;
    std::pair<bool,std::string> result;

    email = get_input("email");
    name = get_input("name");
    phone = get_input("phone");

    result = client->create_record(
        email,
        rmp::info(name,phone));

    if(!result.first)
    {
        std::cerr << "Error: ";
    }

    std::cerr << result.second << std::endl;
}

static void read_command(
    std::shared_ptr<rmp::client>& client)
{
    std::string email;
    std::pair<bool,std::string> result;

    email = get_input("email");

    result = client->read_record(email);

    if(!result.first)
    {
        std::cerr << "Error: ";
    }
    
    std::cerr << result.second << std::endl;
}

static void update_command(
    std::shared_ptr<rmp::client>& client)
{
    std::string email,name,phone;
    rmp::info info;
    std::pair<bool,std::string> result;

    email = get_input("email");
    name = get_input("name(leave blank if unchanged)");
    phone = get_input("phone(leave blank if unchanged)");

    if(name.size() > 0)
    {
        info.set_name(name);
    }

    if(phone.size() > 0)
    {
        info.set_phone(phone);
    }

    result = client->update_record(
        email,info);

    if(!result.first)
    {
        std::cerr << "Error: ";
    }
    
    std::cerr << result.second << std::endl;
}

static void delete_command(
    std::shared_ptr<rmp::client>& client)
{
    std::string email;
    std::pair<bool,std::string> result;

    email = get_input("email");

    result = client->delete_record(email);

    if(!result.first)
    {
        std::cerr << "Error: ";
    }
    
    std::cerr << result.second << std::endl;
}

static void print_help()
{
    std::cerr << std::endl << std::setw(5) << '\0'
              << "client - RMP client shell" 
              << std::endl << std::endl;
    std::cerr << std::setw(5) << '\0' << std::setw(15) 
              << std::left << "create" << std::setw(50)
              << "Create a new record on the server." << std::endl;
    std::cerr << std::setw(5) << '\0' << std::setw(15) 
              << std::left << "read" << std::setw(50)
              << "Read an existing record from the server." << std::endl;
    std::cerr << std::setw(5) << '\0' <<  std::setw(15) 
              << std::left << "update" << std::setw(50)
              << "Update an existing record on the server." << std::endl;
    std::cerr << std::setw(5) << '\0' <<  std::setw(15) 
              << std::left << "create" << std::setw(50)
              << "Delete an existing record from the server." << std::endl;
    std::cerr << std::setw(5) << '\0' << std::setw(15) 
              << std::left << "exit" << std::setw(50)
              << "Exit the client program." << std::endl;
    std::cerr << std::setw(5) << '\0' << std::setw(15) 
              << std::left << "help" << std::setw(50)
              << "Print this message." << std::endl << std::endl;
}