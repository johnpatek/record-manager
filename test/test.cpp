#include "test.h"

void rmp_test::SetUp()
{
    _client = std::make_shared<rmp::client>("127.0.0.1",12345);
}

void rmp_test::TearDown()
{

}

TEST_F(rmp_test,create_test)
{
    std::string email;
    rmp::info info;
    std::pair<bool,std::string> result;

    email = "johnpatek2@gmail.com";
    info.set_name("John");
    info.set_phone("0000000000");
    
    // Test creating a new record
    result = _client->create_record(
        email,
        info);

    EXPECT_TRUE(result.first);

    // Negative test creating a duplicate record
    result = _client->create_record(
        email,
        info);

    EXPECT_FALSE(result.first);    
}

TEST_F(rmp_test,read_test)
{
    std::string good,bad;
    std::pair<bool,std::string> result;
    
    good = "johnpatek2@gmail.com";
    bad = "johnpatek3@gmail.com";
    
    // Test reading existing record
    result = _client->read_record(
        good);

    EXPECT_TRUE(result.first);

    // Negative test reading nonexistent record
    result = _client->read_record(
        bad);

    EXPECT_FALSE(result.first);
}

TEST_F(rmp_test,update_test)
{
    EXPECT_EQ(0,0);
}

TEST_F(rmp_test,delete_test)
{
    std::string good,bad;
    std::pair<bool,std::string> result;
    
    good = "johnpatek2@gmail.com";
    bad = "johnpatek3@gmail.com";
    
    // Test deleting existing record
    result = _client->delete_record(
        good);

    EXPECT_TRUE(result.first);

    // Negative test deleting previously deleted record
    result = _client->delete_record(
        good);

    EXPECT_FALSE(result.first);

    // Negative test deleting nonexistent record
    result = _client->delete_record(
        bad);

    EXPECT_FALSE(result.first);
}