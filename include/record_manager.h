#include <cstdint>
#include <string>
#include <iomanip>
#include <sstream>

namespace rmp
{
    std::string djb_hash(const std::string& data);

    class server
    {
    public:
        server(uint16_t port);
        ~server();
    private:
    };

    class client
    {
    public:
        client(const std::string& host, uint16_t port);
        ~client();
    };
}