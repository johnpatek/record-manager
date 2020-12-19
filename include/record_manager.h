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
#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <uv.h>
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
#include "rmp.pb.h"

namespace rmp
{
    std::string djb_hash(const std::string& data);

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
            int command,
            const record& record);
            
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
        std::set<std::string> _thread_locks;

        std::mutex _thread_locks_mutex;

        void aquire_lock(const std::string& hash);

        void release_lock(const std::string& hash);

        void load_bucket(
            const std::string& hash, 
            bucket& bucket);
        
        void store_bucket(
            const std::string& hash, 
            const bucket& bucket);

        static void uv_read_callback(
            uv_stream_t *client, 
            ssize_t nread, 
            const uv_buf_t *buf);

        static void uv_write_callback(
            uv_write_t *req, 
            int status);

        static void uv_new_connection_callback(
            uv_stream_t *server, 
            int status);

        static void uv_signal_callback(
            uv_signal_t *handle, 
            int signum);

        static void uv_walk_callback(
            uv_handle_t* handle, void* arg);    

        void handle_request(
            const rmp::request& request,
            rmp::response& response);

        response on_create(
            const record& record);

        response on_read(
            const record& record);

        response on_update(
            const record& record);

        response on_delete(
            const record& record);

        std::vector<std::thread> _threads;
        uint16_t _port;
        std::string _root_directory;
        std::shared_ptr<uv_loop_t> _loop;
        uv_tcp_t _handle;
        uv_signal_t _signal;
    };
}