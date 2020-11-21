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

rmp::info::info(const std::string& str)
{
    _info_data = json11::Json(str).object_items();
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
    _info_data.at("name") = name;
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
    _info_data.at("phone") = phone;
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
    _record_data = json11::Json(str).object_items();
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
    _record_data.at("email") = email;
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
    _record_data.at("info") = data;
}

json11::Json rmp::record::to_json() const
{
    return json11::Json(_record_data);
}

std::string rmp::record::to_string() const
{
    return to_json().dump();
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

std::pair<bool,std::string> rmp::client::process_request(
    int command_code, 
    const std::string& body) noexcept
{
    rmp::request_header request_header;
    rmp::response_header response_header;
    std::vector<uint8_t> response_body;
    std::pair<bool,std::string> result;
    
    request_header.command_code = command_code;

    open_connection();

    try
    {
        write_request_header(_socket,&request_header);
        write_request_body(
            _socket,
            reinterpret_cast<const uint8_t* const>(
                body.data()),
            body.size());
        read_response_header(_socket,&response_header);
        response_body.resize(response_header.size);
        read_response_body(
            _socket,
            response_body.data(),
            response_body.size());
        result.first = (response_header.status_code == rmp::status_codes::OK);
        std::move(
            response_body.begin(),
            response_body.end(),
            result.second.begin());
    }
    catch(const std::exception& e)
    {
        result.first = false;
        result.second = e.what();
    }
    
    close_connection();
    
    return result;
}

#define SOCKIO(OP,FD,BUF,LEN,ERROR)                                 \
    if(OP(FD,BUF,LEN) < 0)                                          \
    {                                                               \
        throw(ERROR);                                               \
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