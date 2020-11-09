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