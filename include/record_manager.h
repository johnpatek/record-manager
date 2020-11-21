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
#include <cstdint>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <list>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <json11/json11.hpp>
#if defined(_WIN32)
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#pragma  comment(lib, "ws2_32.lib ")
#else /* Unix */
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#endif


namespace rmp
{
    std::string djb_hash(const std::string& data);

    class info
    {
    public:
        info() = default;

        info(const std::string& str);

        info(const json11::Json& json);

        info(
            const std::string& name,
            const std::string& phone);
        
        std::string get_name() const;

        bool has_name() const;

        void set_name(const std::string& name);

        std::string get_phone() const;

        bool has_phone() const;

        void set_phone(const std::string& phone);

        json11::Json to_json() const;

        std::string to_string() const;

    private:
        json11::Json::object _info_data;
    };

    class record
    {
    public:
        record() = default;

        record(const std::string& str);

        record(const json11::Json& json);

        record(const std::string& email, const info& data);

        bool has_email() const;

        std::string get_email() const;

        void set_email(const std::string& email);

        bool has_info() const;

        info get_info() const;

        void set_info(const info& data);
    
        json11::Json to_json() const;

        std::string to_string() const;

    private:
        json11::Json::object _record_data;
    };
    
    enum command_codes
    {
        CREATE,
        READ,
        UPDATE,
        DELETE
    };

    struct request_header
    {
        uint8_t command_code;
        uint8_t pad0,pad1,pad2;
        uint32_t size;
    };

    enum status_codes
    {
        OK,
        ERROR
    };

    struct response_header
    {
        uint8_t status_code;
        uint8_t pad0,pad1,pad2;
        uint32_t size;
    };

    class client
    {
    public:
        client() = default;

        client(const std::string& host, uint16_t port);

        ~client();

        void open_connection();

        void close_connection();

        void set_address(const std::string& host, uint16_t port);

        std::pair<bool,std::string> create_record(const std::string& email, const info& data);

        std::pair<bool,std::string> read_record(const std::string& email);

        std::pair<bool,std::string> update_record(const std::string& email, const info& data);

        std::pair<bool,std::string> delete_record(const std::string& email);

    private:
        std::pair<bool,std::string> process_request(
            int command_code, 
            const std::string& body) noexcept;
        int _socket;
        sockaddr_in _address;
    };

    class server
    {
    public:
        server() = default;

        server(uint16_t port, const std::string& root_directory);

        ~server();

        void set_port(uint16_t port);

        void set_root_directory(const std::string& root_directory);

        void start();
        
        void run();
        
        void stop();
    
    private:
        void handle_request(int socket);

        std::pair<response_header,std::string> on_create(
            const std::string& request_body);

        std::pair<response_header,std::string> on_read(
            const std::string& request_body);

        std::pair<response_header,std::string> on_update(
            const std::string& request_body);

        std::pair<response_header,std::string> on_delete(
            const std::string& request_body);

        uint16_t _port;
        std::string _root_directory;
        bool _running;
        int _listener;
    };

    const size_t RESPONSE_HEADER_SIZE = sizeof(response_header);
    const size_t REQUEST_HEADER_SIZE = sizeof(request_header);
}