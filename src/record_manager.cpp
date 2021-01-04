/********************************************************************
 * Copyright (c) 2021 John R. Patek
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

static bool validate_request(
    const rmp::request& request, 
    std::string& error_message);

static void allocate_buffer(
    uv_handle_t *handle, 
    size_t suggested_size, 
    uv_buf_t *buf);

static int find_record(
    const rmp::bucket bucket, const rmp::record& record);

std::string rmp::djb_hash(const std::string& data)
{
    std::stringstream hash_string;
    uint32_t hash = 5381;
    for(const char& c : data)
    {
        hash = ((hash << 5) + hash) + c;
    }
    hash_string << std::setbase(16) << hash;
    return hash_string.str();
}

static void write_request(int socket, const rmp::request& request);

static void read_response(int socket, rmp::response& response);

rmp::client::client(const std::string& host, uint16_t port)
{
    set_address(host,port);
}

rmp::client::~client()
{

}

void rmp::client::open_connection()
{
    _socket = socket(PF_INET,SOCK_STREAM,0);
    if(_socket < 0)
    {
        throw std::runtime_error("Failed to open socket");
    }

    if(connect(
        _socket,
        reinterpret_cast<const sockaddr*>(&_address),
        sizeof(_address)) < 0)
    {
        close(_socket);
        throw std::runtime_error("Failed to connect socket");
    }
}

void rmp::client::close_connection()
{
    if(close(_socket) < 0)
    {
        throw std::runtime_error("Failed to close socket");
    }
}

void rmp::client::set_address(
    const std::string& host, uint16_t port)
{
    _address.sin_family = AF_INET;
    inet_pton(AF_INET,host.c_str(),&_address.sin_addr.s_addr);
    _address.sin_port = htons(port);
}

std::pair<bool,std::string> rmp::client::create_record(
    const std::string& email, 
    const rmp::info& data)
{
    rmp::record record;
    rmp::info* info;
    record.set_email(email);
    info = record.mutable_contact();
    info->set_name(data.name());
    info->set_phone(data.phone());
    return process_request(
        rmp::command_codes::CREATE,
        record);
}

std::pair<bool,std::string> rmp::client::read_record(
    const std::string& email)
{
    rmp::record record;
    record.set_email(email);
    return process_request(
        rmp::command_codes::READ,
        record);
}

std::pair<bool,std::string> rmp::client::update_record(
    const std::string& email, 
    const rmp::info& data)
{
    rmp::record record;
    rmp::info* info;
    record.set_email(email);
    info = record.mutable_contact();
    if(data.name().size() > 0)
    {
        info->set_name(data.name());
    }
    if(data.phone().size() > 0)
    {
        info->set_phone(data.phone());
    }
    return process_request(
        rmp::command_codes::UPDATE,
        record);
}

std::pair<bool,std::string> rmp::client::delete_record(
    const std::string& email)
{
    rmp::record record;
    record.set_email(email);
    return process_request(
        rmp::command_codes::DELETE,
        record);
}

std::pair<bool,std::string> rmp::client::process_request(
    int command,
    const rmp::record& record)
{
    rmp::request request;
    rmp::response response;
    std::pair<bool,std::string> result;
    try
    {
        open_connection();
        request.set_command(command);
        *request.mutable_payload() = record;
        write_request(_socket,request);
        read_response(_socket,response);
        close_connection();
        result.first = (response.status() == rmp::status_codes::GOOD);
        result.second = response.payload();
    }
    catch(const std::exception& e)
    {
        result.first = false;
        result.second = e.what();
    }
    return result;
}

rmp::server::server(uint16_t port, const std::string& root_directory)
{
    _loop = std::shared_ptr<uv_loop_t>(uv_default_loop(),[](uv_loop_t * loop)
    {
        uv_loop_close(loop);
    });
    set_port(port);
    set_root_directory(root_directory);
    _loop->data = this;
}

rmp::server::~server()
{

}

void rmp::server::set_port(uint16_t port)
{
    _port = port;
}

void rmp::server::set_root_directory(const std::string& root_directory)
{
    _root_directory = root_directory;
}

void rmp::server::start()
{
    uv_signal_init(_loop.get(),&_signal);
    uv_signal_start(
        &_signal,
        uv_signal_callback,
        SIGINT);
    
    sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", _port, &addr);
    uv_tcp_init(_loop.get(), &_handle);
    uv_tcp_bind(
        &_handle,
        reinterpret_cast<const sockaddr*>(&addr), 0);
    if(uv_listen(
        reinterpret_cast<uv_stream_t*>(&_handle),
        100,uv_new_connection_callback) > 0)
    {
        throw std::runtime_error("Failed to listen");
    }
    uv_run(_loop.get(),UV_RUN_DEFAULT);
}

void rmp::server::stop()
{

}

void rmp::server::uv_read_callback(
    uv_stream_t *client, 
    ssize_t nread, 
    const uv_buf_t *buf)
{
    rmp::server * server = reinterpret_cast<rmp::server*>(
        client->loop->data);
    rmp::request request;
    rmp::response response;
    std::string response_buffer;
    uv_buf_t wrbuf;
    uv_write_t * req = new uv_write_t;
    
    if (nread < 0) 
    {
        if (nread != UV_EOF) 
        {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            uv_close((uv_handle_t*) client, NULL);
        }
    } 
    else if (nread > 0) 
    {
        request.ParseFromArray(buf->base,nread);
        server->handle_request(request,response);
        response_buffer = response.SerializeAsString();
        wrbuf = uv_buf_init(
            const_cast<char*>(
                response_buffer.c_str()), 
            response_buffer.size());
        req->data = client;
        uv_write(req, client, &wrbuf, 1, uv_write_callback);
    }

    if (buf->base) 
    {
        free(buf->base);
    }
}

void rmp::server::uv_write_callback(
    uv_write_t *req, 
    int status)
{
    uv_close(reinterpret_cast<uv_handle_t*>(
        req->data),0);
    delete reinterpret_cast<uv_handle_t*>(
        req->data);    
    delete req;
}

void rmp::server::uv_new_connection_callback(
    uv_stream_t *server, 
    int status)
{
    uv_tcp_t* client = new uv_tcp_t;
    uv_tcp_init(server->loop,client);
    if(uv_accept(
        server,
        reinterpret_cast<uv_stream_t*>(client)) == 0)
    {
        uv_read_start(
            reinterpret_cast<uv_stream_t*>(client),
            allocate_buffer,
            uv_read_callback);
    }
    else
    {
        uv_close(
            reinterpret_cast<uv_handle_t*>(client),0);
    }
    
}

void rmp::server::uv_signal_callback(
    uv_signal_t *handle, 
    int signum)
{
    int result = uv_loop_close(handle->loop);
    if (result == UV_EBUSY)
    {
        uv_walk(handle->loop, uv_walk_callback, NULL);
    }
}

void rmp::server::uv_walk_callback(
    uv_handle_t* handle, void* arg)
{
    uv_close(handle,0);
}

void rmp::server::aquire_lock(const std::string& hash)
{
    std::unique_lock<std::mutex> lock(
        _thread_locks_mutex, std::defer_lock);
    while(_thread_locks.find(hash) != _thread_locks.end())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    lock.lock();
    // Aquire thread lock
    _thread_locks.insert(hash);
    lock.unlock();
}

void rmp::server::release_lock(const std::string& hash)
{
    std::unique_lock<std::mutex> lock(
        _thread_locks_mutex, std::defer_lock);
    lock.lock();
    // Release thread lock
    _thread_locks.erase(hash);
    lock.unlock();
}

void rmp::server::load_bucket(
    const std::string & hash, 
    rmp::bucket& bucket)
{
    std::fstream bucket_stream(
        _root_directory + "/" + hash,
        std::ios::in);
    
    bucket.clear_records();

    if(bucket_stream.good())
    {
        bucket.ParseFromIstream(
            &bucket_stream);
    }
}

void rmp::server::store_bucket(
    const std::string & hash, 
    const rmp::bucket& bucket)
{
    std::fstream bucket_stream(
        _root_directory + "/" + hash,
        std::ios::out);

    bucket.SerializeToOstream(
        &bucket_stream);
}

void rmp::server::handle_request(
    const rmp::request& request,
    rmp::response& response)
{
    std::string error_message;
    if(validate_request(request,error_message))
    {
        switch (request.command())
        {
        case rmp::command_codes::CREATE:
            response = on_create(request.payload());
            break;
        case rmp::command_codes::READ:
            response = on_read(request.payload());
            break;
        case rmp::command_codes::UPDATE:
            response = on_update(request.payload());
            break;
        case rmp::command_codes::DELETE:
            response = on_delete(request.payload());
            break;        
        default:
            break;
        }
    }
    else
    {
        response.set_status(
            rmp::status_codes::BAD);
        *response.mutable_payload() = error_message;
    }
}

rmp::response rmp::server::on_create(
    const rmp::record& record)
{
    std::string hash;
    rmp::bucket bucket;
    rmp::response result;
    hash = djb_hash(record.email());
    aquire_lock(hash);
    load_bucket(hash, bucket);
    if(find_record(bucket,record) == -1)
    {
        *bucket.add_records() = record;
        result.set_status(
            rmp::status_codes::GOOD);
        store_bucket(hash,bucket);
    }
    else
    {
        result.set_status(
            rmp::status_codes::BAD);
        *result.mutable_payload() = "Record already exists";
    }
    release_lock(hash);
    return result;
}

rmp::response rmp::server::on_read(
    const rmp::record& record)
{
    std::string hash;
    rmp::bucket bucket;
    int index;
    rmp::response result;
    hash = djb_hash(record.email());
    aquire_lock(hash);
    load_bucket(hash, bucket);    
    index = find_record(bucket,record);
    if(index != -1)
    {
        bucket.records(index).SerializeToString(
            result.mutable_payload());
    }
    else
    {
        result.set_status(
            rmp::status_codes::BAD);
        *result.mutable_payload() = "Record does not exist";
    }
    release_lock(hash);
    return result;
}

rmp::response rmp::server::on_update(
    const rmp::record& record)
{
    std::string hash;
    rmp::bucket bucket;
    int index;
    rmp::response result;
    hash = djb_hash(record.email());
    aquire_lock(hash);
    load_bucket(hash, bucket);    
    index = find_record(bucket,record);
    if(index != -1)
    {
        result.set_status(
            rmp::status_codes::GOOD);
        bucket.mutable_records(index)->CopyFrom(record);
        store_bucket(hash,bucket);
    }
    else
    {
        result.set_status(
            rmp::status_codes::BAD);
        *result.mutable_payload() = "Record does not exist";
    }
    release_lock(hash);
    return result;
}

rmp::response rmp::server::on_delete(
    const rmp::record& record)
{
    std::string hash;
    rmp::bucket bucket,new_bucket;
    int index;
    rmp::response result;
    hash = djb_hash(record.email());
    aquire_lock(hash);
    load_bucket(hash, bucket);    
    index = find_record(bucket,record);
    if(index != -1)
    {
        result.set_status(
            rmp::status_codes::GOOD);
        for(int i = 0; i < bucket.records_size(); i++)
        {
            if(i != index)
            {
                *new_bucket.add_records() = bucket.records(i);
            }
        }
        store_bucket(hash,new_bucket);
    }
    else
    {
        result.set_status(
            rmp::status_codes::BAD);
        *result.mutable_payload() = "Record does not exist";
    }
    release_lock(hash);
    return result;
}

static bool validate_request(
    const rmp::request& request, 
    std::string& error_message)
{
    bool result(true);
    return result;
}

static int find_record(
    const rmp::bucket bucket, const rmp::record& record)
{
    int result = -1;
    int index = 0;
    while(index < bucket.records_size() && result == -1)
    {
        if(record.email() == bucket.records(index).email())
        {
            result = index;
        }
        index++;
    }
    return result;
}

const size_t READ_BUFFER_SIZE = 1024;

static void write_request(int socket, const rmp::request& request)
{
    request.SerializeToFileDescriptor(socket);
}

static void read_response(int socket, rmp::response& response)
{
    std::vector<uint8_t> response_buffer;
    std::array<uint8_t,READ_BUFFER_SIZE> read_buffer;
    size_t read_size;
    do
    {
        read_size = read(
            socket,
            read_buffer.data(),
            read_buffer.size());
        response_buffer.insert(
            response_buffer.end(),
            read_buffer.begin(),
            read_buffer.begin() + read_size);
    } while (read_size == 1024);
    if(response_buffer.size() > 0)
    {
        response.ParseFromArray(
            response_buffer.data(),
            response_buffer.size());
    }
}

static void allocate_buffer(
    uv_handle_t *handle, 
    size_t suggested_size, 
    uv_buf_t *buf)
{
    buf->base = reinterpret_cast<char*>(
        malloc(
            suggested_size));
    buf->len = suggested_size;
}
