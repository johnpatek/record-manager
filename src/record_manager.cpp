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

static bool validate_request(
    const rmp::request& request, 
    std::string& error_message);

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

static void read_request(int socket, rmp::request& request);

static void write_request(int socket, const rmp::request& request);

static void read_response(int socket, rmp::response& response);

static void write_response(int socket, const rmp::response& response);

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
    set_port(port);
    set_root_directory(root_directory);
}

rmp::server::~server()
{
    if(_running)
    {
        stop();
    }
}

void rmp::server::set_port(uint16_t port)
{
    _port = htons(port);
}

void rmp::server::set_root_directory(const std::string& root_directory)
{
    _root_directory = root_directory;
}

void rmp::server::start()
{
    _listener = socket(PF_INET,SOCK_STREAM,0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = _port;

    if(_listener < 0)
    {
        throw std::runtime_error("Failed to open socket");
    }

    if(bind(
        _listener,
        reinterpret_cast<sockaddr*>(&addr),
        sizeof(addr)) < 0)
    {
        throw std::runtime_error("Failed to bind socket");
    }

    if(listen(_listener,100) < 0)
    {
        throw std::runtime_error("Failed to listen to socket");
    }

    size_t thread_count = std::max<size_t>(
        4,std::thread::hardware_concurrency());
    
    _running = true;

    while(_threads.size() < thread_count)
    {
        _threads.push_back(std::thread([this]
        {
            this->run();
        }));
        _threads.back().detach();
    }
}

void rmp::server::run()
{
    int connection;
    bool loop(true);
    while(loop)
    {
        connection = accept(_listener,nullptr,nullptr);
        if(connection < 0)
        {
            loop = false;
        }
        else
        {
            handle_request(connection);   
            close(connection);
        }
    }
}

void rmp::server::stop()
{
    close(_listener);
    _running = false;
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

void rmp::server::handle_request(int socket)
{
    rmp::request request;
    rmp::response response;
    std::string error_message;
    read_request(socket,request);
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
    write_response(socket,response);
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

static void read_request(int socket, rmp::request& request)
{
    std::vector<uint8_t> request_buffer;
    std::array<uint8_t,READ_BUFFER_SIZE> read_buffer;
    size_t read_size;
    do
    {
        read_size = read(
            socket,
            read_buffer.data(),
            read_buffer.size());
        request_buffer.insert(
            request_buffer.end(),
            read_buffer.begin(),
            read_buffer.begin() + read_size);
    } while (read_size == 1024);
    request.ParseFromArray(
        request_buffer.data(),
        request_buffer.size());
}

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
    response.ParseFromArray(
        response_buffer.data(),
        response_buffer.size());
}

static void write_response(int socket, const rmp::response& response)
{
    response.SerializeToFileDescriptor(socket);
}