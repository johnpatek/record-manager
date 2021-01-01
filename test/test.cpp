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
    EXPECT_EQ(0,0);
}

TEST_F(rmp_test,create_test)
{
    EXPECT_EQ(0,0);
}

TEST_F(rmp_test,read_test)
{
    EXPECT_EQ(0,0);
}

TEST_F(rmp_test,update_test)
{
    EXPECT_EQ(0,0);
}

TEST_F(rmp_test,delete_test)
{
    EXPECT_EQ(0,0);
}