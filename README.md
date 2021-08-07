# snowflake uid generator

## Description
[snowflake uid algorithm](https://github.com/twitter-archive/snowflake) is a uid generator algorithm for generating unique ID numbers used in 
distributed system from Twitter. This repo is cpp implemented version. It is referenced from [baidu's uid generator](https://github.com/baidu/uid-generator)
and [this repo](https://github.com/johnhuang-cn/snowflake-uid), follow the non-db-dependency design, this make this repo can also be used as a lib instead of only uid service. 
This generator supports to customize work id bits, datacenter id bits and sequence id bits. There are three constructors in this generator, 
first is the origin version, 5bit for worker id, 5bit for datacenter id, 12bit for sequence id and the last bits for delta timestamp;
second uses the 16bit value as work id without datacenter id, it's designed to suit k8s pod CIDR value, when CIDR is /16, we could use
last 2byte value of ip address as worker id, at the same time we could ensure the uid generated in each service is unique, 
and default sequence id length is 8, you could increase this value to get higher QPS, but in my opinion performance bottlenecks of business code is not usually on id generator, 
of course when this repo is used as uid service I advice set sequence id length as 15. The third constructor is fully customize generator, 
you need give work id length, sequence id length, datacenter id length.  

And this repo provides utils for generate worker id with ip address in unix-like system. Enjoy it;


## 中文描述
雪花算法是twitter发布的分布式uid生成算法，这个仓库是cpp实现的版本了，借鉴了[百度的uid算法](https://github.com/baidu/uid-generator) 和[这个仓库](https://github.com/johnhuang-cn/snowflake-uid) ，
并延续了其不依赖数据库的设计，这使得这个库可以作为一个库来使用而不仅仅是作为发号服务。这个生成器有三个构成函数.第一个构成函数，使用最原始的版本，work id占五位，数据中心id占五位，生成序列号占12位。第二个
构成函数，使用16位作为worker id 没有数据中心id，这样做的目的是为了适配k8s的CIDR配置，当CIDR配置为16时，可以直接使用ip的最后两个字节作为worker id。
同时还可以集群中所有服务生成的id保持一致，默认的生成序列id为8, 你可以提高这个值来获得更高的QPS，不过我认为业务代码的瓶颈并不会在id生成上面，当然你要将这个服务作为发号服务，
我建议将生成序列的长度改为15。第三个构造函数则需要自行指定worker id长度, 生产序列长度，数据中心长度。

同时这个仓库还提供ip生成worker id的工具，满足在类unix环境下使用。

## Benchmark
```
Run on (8 X 1400 MHz CPU s)
CPU Caches:
L1 Data 32 KiB (x4)
L1 Instruction 32 KiB (x4)
L2 Unified 256 KiB (x4)
L3 Unified 6144 KiB (x1)
Load Average: 1.66, 2.01, 1.96
--------------------------------------------------------------------
Benchmark                          Time             CPU   Iterations
--------------------------------------------------------------------
BM_snowflake_id_generator       3899 ns         3852 ns       182900
```

## Usage

1. use second constructor in `main.cpp`

```c++
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
```

2. use default constructor
```c++
SnowflakeFactory<1577808000000L, std::mutex> factory(nullptr);
```

3. use third constructor
```c++
SnowflakeFactory<1577808000000L, std::mutex> factory(6, 2, 14);
```

## installation
```
git clone git@github.com:Catelemmon/snowflake-cpp-k8s.git
cd snowflake-cpp-k8s
make install 
```

PS: after installation, please use `#include <snowflake/snowflake.h>`

## Compile and run

1. install [google test(gtest)](https://github.com/google/googletest) and [google benchmark](https://github.com/google/benchmark) ,
2. download and compile, specific where google test and google benchmark execute cmake 
```
git clone git@github.com:Catelemmon/snowflake-cpp-k8s.git
cd snowflake-cpp-k8s
mkdir build && cd build
cmake .. -DGTEST_INCLUDE_DIR=/usr/local/include -DGTEST_LIB_DIR=/usr/local/lib
make -j 4
```

## LICENSE

Licenced by [MIT](https://github.com/Ice-Hazymoon/MikuTools/blob/master/LICENSE)
