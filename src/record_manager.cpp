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

static void read_request_header(
    int socket,
    rmp::request_header * const request_header);

static void read_request_body(
    int socket,
    uint8_t * const request_buffer,
    size_t request_size);

static void write_request_header(
    int socket,
    const rmp::request_header * const request_header);

static void write_request_body(
    int socket,
    const uint8_t * const request_buffer,
    size_t request_size);

static void read_response_header(
    int socket,
    rmp::response_header * const request_header);

static void read_response_body(
    int socket,
    uint8_t * const response_buffer,
    size_t request_size);

static void write_response_header(
    int socket,
    const rmp::response_header * const response_header);

static void write_response_body(
    int socket,
    const uint8_t * const response_buffer,
    size_t response_size);

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

bool rmp::file_exists(const std::string& path)
{
#ifdef _WIN32
    // TODO: implement for Windows
#else
    struct stat buffer;   
    return (stat (path.c_str(), &buffer) == 0); 
#endif
}

static json11::Json parse_json(const char * const buffer)
{
    std::string error;
    json11::Json result = json11::Json::parse(
        buffer,
        error,
        json11::JsonParse::STANDARD);
    if(error.size() > 0)
    {
        throw std::runtime_error(error);
    }
    return result;
}

static json11::Json parse_json(const std::string& data)
{
    return parse_json(data.c_str());
}

rmp::info::info(const std::string& str)
{
    _info_data = parse_json(str).object_items();
}

rmp::info::info(const json11::Json& json)
{
    _info_data = json.object_items();
}

rmp::info::info(
    const std::string& name,
    const std::string& phone)
{
    _info_data["name"] = name;
    _info_data["phone"] = phone;
}

std::string rmp::info::get_name() const
{
    return _info_data.at("name").string_value();
}

bool rmp::info::has_name() const
{
    return _info_data.find("name") != _info_data.end();
}

void rmp::info::set_name(const std::string& name)
{
    if(has_name())
    {
        _info_data.at("name") = name;
    }
    else
    {
        _info_data.insert(
        {
            "name",
            name
        });
    }
}

std::string rmp::info::get_phone() const
{
    return _info_data.at("phone").string_value();
}

bool rmp::info::has_phone() const
{
    return _info_data.find("phone") != _info_data.end();
}

void rmp::info::set_phone(const std::string& phone)
{
    if(has_phone())
    {
        _info_data.at("phone") = phone;
    }
    else
    {
        _info_data.insert(
        {
            "phone",
            phone
        });
    }
}

json11::Json rmp::info::to_json() const
{
    return json11::Json(_info_data);
}

std::string rmp::info::to_string() const
{
    return to_json().dump();
}

rmp::record::record(const std::string& str)
{
    _record_data = parse_json(str).object_items();
}

rmp::record::record(const json11::Json& json)
{
    _record_data = json.object_items();
}

rmp::record::record(const std::string& email, const info& data)
{
    _record_data["email"] = email;
    _record_data["info"] = data.to_json();
}

bool rmp::record::has_email() const
{
    return _record_data.find("email") != _record_data.end();
}

std::string rmp::record::get_email() const
{
    return _record_data.at("email").string_value();
}

void rmp::record::set_email(const std::string& email)
{
    if(has_email())
    {
        _record_data.at("email") = email;
    }
    else
    {
        _record_data.insert(
        {
            "email",
            email
        });
    }
}

bool rmp::record::has_info() const
{
    return _record_data.find("info") != _record_data.end();
}

rmp::info rmp::record::get_info() const
{
    return rmp::info(_record_data.at("info"));
}

void rmp::record::set_info(const rmp::info& data)
{
    if(has_info())
    {
        _record_data.at("info") = data;
    }
    else
    {
        _record_data.insert(
        {
            "info",
            data
        });
    }
}

json11::Json rmp::record::to_json() const
{
    return json11::Json(_record_data);
}

std::string rmp::record::to_string() const
{
    return to_json().dump();
}

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
    rmp::record record(email,data);
    return process_request(
        rmp::command_codes::CREATE,
        record.to_string());
}

std::pair<bool,std::string> rmp::client::read_record(
    const std::string& email)
{
    rmp::record record;
    record.set_email(email);
    return process_request(
        rmp::command_codes::READ,
        record.to_string());
}

std::pair<bool,std::string> rmp::client::update_record(
    const std::string& email, 
    const rmp::info& data)
{
    rmp::record record(email,data);
    return process_request(
        rmp::command_codes::UPDATE,
        record.to_string());
}

std::pair<bool,std::string> rmp::client::delete_record(
    const std::string& email)
{
    rmp::record record;
    record.set_email(email);
    return process_request(
        rmp::command_codes::DELETE,
        record.to_string());
}

std::pair<bool,std::string> rmp::client::process_request(
    int command_code, 
    const std::string& body)
{
    rmp::request_header request_header;
    rmp::response_header response_header;
    std::vector<uint8_t> response_body;
    std::pair<bool,std::string> result;
    
    request_header.command_code = command_code;
    request_header.size = body.size();

    open_connection();

    try
    {
        result.first = true;
        write_request_header(_socket,&request_header);
        write_request_body(
            _socket,
            reinterpret_cast<const uint8_t* const>(
                body.data()),
            body.size());
        read_response_header(_socket,&response_header);
        if(response_header.size > 0)
        {
            response_body.resize(response_header.size);
            read_response_body(
                _socket,
                response_body.data(),
                response_body.size());
            result.first = (response_header.status_code 
                == rmp::status_codes::OK);
            std::move(
                response_body.begin(),
                response_body.end(),
                std::back_inserter(result.second));
        }
    }
    catch(const std::exception& e)
    {
        result.first = false;
        result.second = e.what();
    }
    
    close_connection();
    
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
}

void rmp::server::run()
{
    size_t thread_count = std::max<size_t>(
        4,std::thread::hardware_concurrency());
    
    std::function<void()> server_loop = [&]
    {
        int connection;
        while(_running)
        {
            connection = accept(_listener,nullptr,nullptr);
            if(connection < 0)
            {
                _running = false;
            }
            else
            {
                handle_request(connection);   
                close(connection);
            }
        }
    };
    
    while(_threads.size() < thread_count)
    {
        _running = true;   
        _threads.push_back(
            std::move(
                std::thread(
                    server_loop)));    
    }
}

void rmp::server::stop()
{
    _running = false;
    close(_listener);
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

const size_t BUFFER_SIZE = 1024;

void rmp::server::load_bucket(
    const std::string & hash, 
    std::vector<rmp::record>& records)
{
    std::cerr << "loading" << std::endl;
    std::stringstream json_string;
    std::array<char,BUFFER_SIZE> read_buffer;
    json11::Json::array array_data;
    std::string path(
        _root_directory + "/" + hash);
    size_t read_size;
    if(file_exists(path))
    {
        std::fstream bucket_file(
            path,
            std::ios::in);
        
        while(bucket_file.good())
        {
            std::cerr << "reading" << std::endl;
            bucket_file.read(
                read_buffer.data(),
                read_buffer.size());
            read_size = bucket_file.gcount();
            json_string.write(
                read_buffer.data(),
                read_size);
        }

        array_data = parse_json(
            json_string.str()).array_items();
        
        records.reserve(array_data.size());
        for(json11::Json& record_data : array_data)
        {
            records.push_back(
                record_data);
        }
    }
}

void rmp::server::store_bucket(
    const std::string & hash, 
    const std::vector<rmp::record>& records)
{
    std::fstream bucket_file(
        _root_directory + "/" + hash,
        std::ios::out);
    json11::Json::array array_data;
    
    for(const rmp::record& record : records)
    {
        array_data.push_back(
            record.to_json());
    }

    bucket_file << json11::Json(array_data).dump();
}

void rmp::server::handle_request(int socket)
{
    rmp::request_header request_header;
    std::vector<uint8_t> request_body;
    std::pair<rmp::response_header,std::string> response;

    read_request_header(
        socket,
        &request_header);

    request_body.resize(request_header.size);
    
    read_request_body(
        socket,
        request_body.data(),
        request_body.size());

    std::cerr.write(
        reinterpret_cast<const char*>(
            request_body.data()),
        request_body.size());
    std::cerr << std::endl;

    switch (request_header.command_code)
    {
        case rmp::command_codes::CREATE:
            response = on_create(
                rmp::record(
                    std::string(
                        reinterpret_cast<char*>(
                            request_body.data()),
                        request_body.size())));
            break;
        case rmp::command_codes::READ:
            response = on_read(
                rmp::record(
                    std::string(
                        reinterpret_cast<char*>(
                            request_body.data()),
                        request_body.size())));
            break;
        case rmp::command_codes::UPDATE:
            response = on_update(
                rmp::record(
                    std::string(
                        reinterpret_cast<char*>(
                            request_body.data()),
                        request_body.size())));
            break;
        case rmp::command_codes::DELETE:
            response = on_delete(
                rmp::record(
                    std::string(
                        reinterpret_cast<char*>(
                            request_body.data()),
                        request_body.size())));
            break;
        default:
            response.second = "Invalid command code";
            response.first.size = response.second.size();
            response.first.status_code = rmp::status_codes::ERROR;
            break;
    }

    write_response_header(socket,&response.first);

    write_response_body(
        socket,
        reinterpret_cast<const uint8_t* const>(
            response.second.data()),
        response.second.size());
}

enum request_types
{
    CREATE_RECORD,
    READ_RECORD,
    UPDATE_RECORD,
    DELETE_RECORD
};

static bool validate_structure(
    int request_type,
    const rmp::record& record, 
    std::string& error);

static std::vector<rmp::record>::iterator find_record(
    std::vector<rmp::record> bucket, const rmp::record& record);

std::pair<rmp::response_header, std::string> rmp::server::on_create(
    const rmp::record& request_body)
{
    std::string hash;
    std::vector<rmp::record> bucket;
    std::pair<rmp::response_header, std::string> result;
    std::cerr << "validate" <<std::endl;
    if(validate_structure(
        request_types::CREATE_RECORD,
        request_body,
        result.second))
    {
        result.first.status_code = rmp::status_codes::OK;
    }
    else
    {
        result.first.status_code = rmp::status_codes::ERROR;
    }
    
    if(result.first.status_code == rmp::status_codes::OK)
    {
        hash = rmp::djb_hash(
            request_body.get_email());
        std::cerr << "lock" << std::endl;
        aquire_lock(hash);
        std::cerr << "load" << std::endl;
        load_bucket(hash,bucket);
        std::cerr << "find" << std::endl;
        if(find_record(bucket,request_body) == bucket.end())
        {
            bucket.push_back(request_body);
            std::cerr << "store" << std::endl;
            store_bucket(hash,bucket);
        }
        else
        {
            result.first.status_code = rmp::status_codes::ERROR;
            result.second = "Record already exists.";
        }
        std::cerr << "release" << std::endl;
        release_lock(hash);
    }
    
    result.first.size = result.second.size();
    return result;
}

std::pair<rmp::response_header, std::string> rmp::server::on_read(
    const rmp::record& request_body)
{
    std::string hash;
    std::vector<rmp::record> bucket;
    std::pair<rmp::response_header, std::string> result;
    std::vector<rmp::record>::iterator record;

    if(validate_structure(
        request_types::READ_RECORD,
        request_body,
        result.second))
    {
        result.first.status_code = rmp::status_codes::OK;
    }
    else
    {
        result.first.status_code = rmp::status_codes::ERROR;
    }
    
    if(result.first.status_code == rmp::status_codes::OK)
    {
        hash = rmp::djb_hash(
            request_body.get_email());
        aquire_lock(hash);
        load_bucket(hash,bucket);
        record = find_record(bucket,request_body);
        if(record != bucket.end())
        {
            result.second = record->to_string();
        }
        else
        {
            result.first.status_code = rmp::status_codes::ERROR;   
            result.second = "Record does not exist.";
        }
        release_lock(hash);
    }
    
    result.first.size = result.second.size();
    return result;
}

std::pair<rmp::response_header, std::string> rmp::server::on_update(
    const rmp::record& request_body)
{
    std::string hash;
    std::vector<rmp::record> bucket;
    std::pair<rmp::response_header, std::string> result;
    std::vector<rmp::record>::iterator record;
    rmp::info record_info,new_info;

    if(validate_structure(
        request_types::UPDATE_RECORD,
        request_body,
        result.second))
    {
        result.first.status_code = rmp::status_codes::OK;
    }
    else
    {
        result.first.status_code = rmp::status_codes::ERROR;
    }
    
    if(result.first.status_code == rmp::status_codes::OK)
    {
        hash = rmp::djb_hash(
            request_body.get_email());
        new_info = request_body.get_info();
        aquire_lock(hash);
        load_bucket(hash,bucket);
        record = find_record(bucket,request_body);
        record_info = record->get_info();
        if(record != bucket.end())
        {
            if(new_info.has_name())
            {
                record_info.set_name(
                    new_info.get_name());
            }

            if(new_info.has_phone())
            {
                record_info.set_phone(
                    new_info.get_phone());
            }

            record->set_info(record_info);
        }
        else
        {
            result.first.status_code = rmp::status_codes::ERROR;   
            result.second = "Record does not exist.";
        }
        release_lock(hash);
    }
    
    result.first.size = result.second.size();
    return result;
}

std::pair<rmp::response_header, std::string> rmp::server::on_delete(
    const rmp::record& request_body)
{
    std::string hash;
    std::vector<rmp::record> bucket;
    std::pair<rmp::response_header, std::string> result;
    std::vector<rmp::record>::iterator record;

    if(validate_structure(
        request_types::DELETE_RECORD,
        request_body,
        result.second))
    {
        result.first.status_code = rmp::status_codes::OK;
    }
    else
    {
        result.first.status_code = rmp::status_codes::ERROR;
    }
    
    if(result.first.status_code == rmp::status_codes::OK)
    {
        hash = rmp::djb_hash(
            request_body.get_email());
        aquire_lock(hash);
        load_bucket(hash,bucket);
        record = find_record(bucket,request_body);
        if(record != bucket.end())
        {
            bucket.erase(record);
            store_bucket(hash,bucket);
        }
        else
        {
            result.first.status_code = rmp::status_codes::ERROR;   
            result.second = "Record does not exist.";
        }
        release_lock(hash);
    }
    
    result.first.size = result.second.size();
    return result;    
}

#define SOCKIO(OP,FD,BUF,LEN,ERROR)                                 \
    if(OP(FD,BUF,LEN) < 0)                                          \
    {                                                               \
        throw(ERROR);                                               \
    }

static void read_request_header(
    int socket,
    rmp::request_header * const request_header)
{
    SOCKIO(
        read,
        socket,
        request_header,
        rmp::REQUEST_HEADER_SIZE,
        "Failed to read request header")
}

static void read_request_body(
    int socket,
    uint8_t * const request_buffer,
    size_t request_size)
{
    SOCKIO(
        read,
        socket,
        request_buffer,
        request_size,
        "Failed to read request body")
}

static void write_request_header(
    int socket,
    const rmp::request_header * const request_header)
{
    SOCKIO(
        write,
        socket,
        request_header,
        rmp::REQUEST_HEADER_SIZE,
        "Failed to write request header")
}

static void write_request_body(
    int socket,
    const uint8_t * const request_buffer,
    size_t request_size)
{
    SOCKIO(
        write,
        socket,
        request_buffer,
        request_size,
        "Failed to write request body")
}

static void read_response_header(
    int socket,
    rmp::response_header * const response_header)
{
    SOCKIO(
        read,
        socket,
        response_header,
        rmp::RESPONSE_HEADER_SIZE,
        "Failed to read response header")
}

static void read_response_body(
    int socket,
    uint8_t * const response_buffer,
    size_t response_size)
{
    SOCKIO(
        read,
        socket,
        response_buffer,
        response_size,
        "Failed to read response body")
}

static void write_response_header(
    int socket,
    const rmp::response_header * const response_header)
{
    SOCKIO(
        write,
        socket,
        response_header,
        rmp::RESPONSE_HEADER_SIZE,
        "Failed to write response header")
}

static void write_response_body(
    int socket,
    const uint8_t * const response_buffer,
    size_t response_size)
{
    SOCKIO(
        write,
        socket,
        response_buffer,
        response_size,
        "Failed to write response body")   
}

static bool validate_structure(
    int request_type,
    const rmp::record& record, 
    std::string& error)
{
    bool result;

    result = record.has_email();

    if(!result)
    {
        error = "Email required.";
    }

    if(result 
        && (request_type == request_types::CREATE_RECORD 
        || request_type == request_types::UPDATE_RECORD))
    {
        rmp::info info;
        if(record.has_info())
        {
            info = record.get_info();
        }
        else
        {
            result = false;
            error = "Info required.";
        }

        if(result && (request_type == request_types::CREATE_RECORD))
        {
            result = info.has_name() && info.has_phone();
            error = (result)?"":"Name and phone required.";
        }

        if(result && (request_type == request_types::UPDATE_RECORD))
        {
            result = info.has_name() || info.has_phone();
            error = (result)?"":"Name or phone required.";
        }
    }

    return result;
}

static std::vector<rmp::record>::iterator find_record(
    std::vector<rmp::record> bucket, const rmp::record& record)
{
    std::vector<rmp::record>::iterator result,it;
    result = bucket.end();
    it = bucket.begin();
    while(
        (result == bucket.end()) 
        && (it != bucket.end()))
    {
        if(record.get_email() == it->get_email())
        {
            result = it;
        }
    }
    return result;
}