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