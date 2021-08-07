//
// Created by cicada on 2021/8/8.
//

#include <iostream>
#include "utils.h"
#include "snowflake.h"

int main() {
    auto hostname = get_hostname();
    auto ip_addr = look_up_ip(hostname);
    auto worker_id = ip_2_worker_id(ip_addr, K8S_CIDR_16);
    SnowflakeFactory<1577808000000L, std::mutex> factory;
    factory.init_factory(worker_id, 0);
    auto sfid = factory.next_id();
    std::cout << sfid;
}