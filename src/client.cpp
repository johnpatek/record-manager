#include "record_manager.h"

class json_info : public json11::Json::object
{

};

int main(int argc, const char ** argv)
{
    json11::Json::object record;
    json_info info;
    info["email"] = "johnpatek2@gmail.com";
    record["info"] = info;
    std::cerr << json11::Json(record).dump() << std::endl;
    return 0;
}