//
// Created by cicada on 2021/8/6.
//

#ifndef SNOWFLAKE_CPP_UTILS_H
#define SNOWFLAKE_CPP_UTILS_H

#include <iostream>
#include <cstdint>
#include <chrono>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex>
#include <sstream>

enum K8S_CIDR_TYPE {
    K8S_CIDR_16,
    K8S_CIDR_24
};

unsigned long long now_timestamp(){
    using namespace std;
    auto now = chrono::duration_cast< chrono::milliseconds >(
            chrono::system_clock::now().time_since_epoch()
    );
    return now.count();
}

std::string get_hostname(){
    auto hostname_ptr = std::getenv("HOSTNAME");
    if(hostname_ptr == nullptr){
        char hostname_buf[128] = {};
        if (gethostname(hostname_buf, 128) < 0){
            throw std::runtime_error("failed to get hostname");
        }
        return std::string(hostname_buf);
    } else{
        return std::string(hostname_ptr);
    }
}

std::string look_up_ip(const std::string& hostname){
    std::string ip_str;
    struct addrinfo hint{}, *addrinfos, *loop_ptr;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    int ret = getaddrinfo(hostname.c_str(), nullptr, &hint, &addrinfos);
    if (ret != 0){
        std::ostringstream msg;
        msg << "getaddrinfo " << "hostname" << "\"" << hostname << "\" with err: " << gai_strerror(ret);
        throw std::runtime_error(msg.str());
    }

    for(loop_ptr = addrinfos; loop_ptr != nullptr; loop_ptr = loop_ptr->ai_next){
        char ip_cstr[16] = {};
        if(loop_ptr->ai_family == AF_INET){
            // IPv4 address
            auto *ipv4 = (struct sockaddr_in*)loop_ptr->ai_addr;
            inet_ntop(loop_ptr->ai_family, &(ipv4->sin_addr), ip_cstr, INET6_ADDRSTRLEN);
            ip_str = std::string(ip_cstr);
            break;
        }
    }
    freeaddrinfo(addrinfos);
    return ip_str;
}

uint64_t ip_2_worker_id(const std::string& ip,  K8S_CIDR_TYPE cidr_type){
    std::regex ip_pattern(R"(^((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})(\.((2(5[0-5]|[0-4]\d))|[0-1]?\d{1,2})){3}$)");
    if (!std::regex_match(ip, ip_pattern)){
        throw std::runtime_error("ip address is not valid");
    }
    std::regex dot_split("\\."); // . dot
    std::vector<std::string> pieces(std::sregex_token_iterator(ip.begin(), ip.end(), dot_split, -1),
                       std::sregex_token_iterator());
    uint64_t int_ip[4] = {};
    for (int i = 0; i < 4; i++){
         int_ip[i] = std::stoi(pieces[i]);
    }
    switch (cidr_type) {
        case K8S_CIDR_16:
            return (int_ip[2] << 8) + int_ip[3];
        case K8S_CIDR_24:
            return (int_ip[3] << 16) + (int_ip[2] << 8) + int_ip[3];
    }
}

#endif //SNOWFLAKE_CPP_UTILS_H
