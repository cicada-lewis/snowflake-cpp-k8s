//
// Created by cicada on 2021/8/7.
//

#include <benchmark/benchmark.h>

#include "../snowflake.h"


static void BM_snowflake_id_generator(benchmark::State& state){
    auto hostname = get_hostname();
    auto ip_addr = look_up_ip(hostname);
    auto worker_id = ip_2_worker_id(ip_addr, K8S_CIDR_16);
    SnowflakeFactory<1628313123409L, std::mutex> factory;
    factory.init_factory(worker_id, 0);
    for (auto _ : state){
        factory.next_id();
    }
}

BENCHMARK(BM_snowflake_id_generator);
BENCHMARK_MAIN();