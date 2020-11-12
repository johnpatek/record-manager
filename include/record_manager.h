#include <cstdint>
#include <string>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <list>
#include <unordered_map>
#include "json.h"

namespace rmp
{
    std::string djb_hash(const std::string& data);

    BEGIN_JSON_OBJECT(record)
        JSON_PRIMITIVE(email,"email",std::string);
        JSON_PRIMITIVE(first,"first",std::string);
        JSON_PRIMITIVE(last,"last",std::string);
        JSON_PRIMITIVE(phone,"phone",std::string);
        JSON_KEY(email);
    END_JSON_OBJECT

    BEGIN_JSON_OBJECT(record_list)
        JSON_ARRAY_ITEMS(records,"records",record);
    END_JSON_OBJECT

    void f()
    {
        record_list rl;
        parse_json<record_list>("{}");
    }
    enum command_codes
    {
        CREATE,
        READ,
        UPDATE,
        DELETE
    };

    class request_header
    {
        uint8_t command_code;
    };

    enum status_codes
    {
        Ok,
        ERROR
    };

    class response_header
    {

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
        std::pair<std::string,record_list> _cache;
    };

    class client
    {
    public:
        client(const std::string& host, uint16_t port);
        ~client();

        std::pair<bool,std::string> create_record(const record& data);

        std::pair<bool,std::string> read_record(std::string& email);

        std::pair<bool,std::string> update_record(const record& data);

        std::pair<bool,std::string> delete_record(const std::string& email);

    private:
        std::pair<response_header,std::string> send_request(
            const request_header& header, 
            const std::string& body);
    };
}