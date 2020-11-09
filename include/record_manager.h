#include <cstdint>
#include <string>
#include <iomanip>
#include <sstream>
#include <list>
#include <unordered_map>


namespace rmp
{
    std::string djb_hash(const std::string& data);

    class record
    {
    private:
        std::string _first_name;
        std::string _last_name;
        std::string _phone;
    };

    class request
    {

    };

    class response
    {

    };

    class server
    {
    public:
        server(uint16_t port);
        ~server();
    private:
        struct hash
        {
            std::string operator()(const std::string& data)
            {
                return djb_hash(data);
            }
        };
        std::unordered_map<std::string,std::string,hash> _cache;    
    };

    class client
    {
    public:
        client(const std::string& host, uint16_t port);
        ~client();
    };
}