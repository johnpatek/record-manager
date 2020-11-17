#include <cstdint>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <list>
#include <unordered_map>
#include <json11/json11.hpp>

namespace rmp
{
    std::string djb_hash(const std::string& data);

    class info
    {
    public:
        info(const std::string& str);

        info(const json11::Json& json);

        info(
            const std::string& name,
            const std::string& phone);
        
        std::string& get_name() const;

        bool has_name() const;

        void set_name(const std::string& name);

        std::string& get_phone() const;

        bool has_phone() const;

        void set_phone(const std::string& phone);

    private:
        json11::Json::object _data;
    };

    class record
    {
    public:
        record(const std::string& str);

        record(const json11::Json& json);
    private:

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
    };

    enum status_codes
    {
        Ok,
        ERROR
    };

    struct response_header
    {
        uint8_t status_code;
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
        int _listener;
    };

    class client
    {
    public:
        client(const std::string& host, uint16_t port);
        ~client();

        std::pair<bool,std::string> create_record(const std::string& email, const info& data);

        std::pair<bool,std::string> read_record(const std::string& email);

        std::pair<bool,std::string> update_record(const std::string& email, const info& data);

        std::pair<bool,std::string> delete_record(const std::string& email);

    private:
        std::pair<response_header,std::string> send_request(
            const request_header& header, 
            const std::string& body);
    };
}