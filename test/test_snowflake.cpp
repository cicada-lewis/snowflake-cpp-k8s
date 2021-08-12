//
// Created by cicada on 2021/8/7.
//

#include <unordered_set>
#include <cstdint>
#include <gtest/gtest.h>
#include "../snowflake.h"


static std::unordered_set<uint64_t> duper;



class SnowflakeFactoryTest : public ::testing::Test{
protected:
    void SetUp() override{
        auto hostname = get_hostname();
        auto ip_addr = look_up_ip(hostname);
        auto worker_id = ip_2_worker_id(ip_addr, K8S_CIDR_16);
        factory.init_factory(worker_id, 0);
        factory2.init_factory(worker_id, 0);
    }

    SnowflakeFactory<1628313123409L, std::mutex> factory;
    SnowflakeFactory<1628313123409L, std::mutex> factory2;

};

TEST_F(SnowflakeFactoryTest, IsUnique) {
    for (u_long i = 0; i < 2900000; i++){
        auto id = factory.next_id();
        ASSERT_EQ(duper.count(id), 0);

        duper.insert(id);
    }
}

TEST_F(SnowflakeFactoryTest, IsPositive) {
    for (u_long i = 0; i < 290; i++){
        auto id = factory.next_id();
        std::cout << id << std::endl;
        ASSERT_GT(id, 0);

        duper.insert(id);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

