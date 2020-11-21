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

    class server
    {
    public:
    
        server(uint16_t port, const std::string& root_directory);
        
        ~server();

        void start();
        
        void run();
        
        void stop();
    
    private:
        bool _running;
        int _listener;
    };

    class client
    {
    public:
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

    const size_t RESPONSE_HEADER_SIZE = sizeof(response_header);
    const size_t REQUEST_HEADER_SIZE = sizeof(request_header);
}