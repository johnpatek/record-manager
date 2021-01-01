#include "record_manager.h"
#include <gtest/gtest.h>
#include <random>
#include <chrono>
#include <type_traits>

class rmp_test : public ::testing::Test
{
protected:
    virtual void SetUp(void) override;
    virtual void TearDown(void) override;
    std::shared_ptr<rmp::client> _client;
};