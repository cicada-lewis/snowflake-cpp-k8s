//
// Created by cicada on 2021/8/5.
//

#ifndef SNOWFLAKE_CPP_SNOWFLAKE_H
#define SNOWFLAKE_CPP_SNOWFLAKE_H

#include <cstdint>
#include <stdexcept>
#include <mutex>
#include <sstream>
#include "utils.h"

// we do not recommend use it with empty locker
class snowflake_empty_locker{
public:
    void lock(){}
    void unlock(){}
};


template<uint64_t Twepoch, typename Lock = snowflake_empty_locker>
class SnowflakeFactory {
    // start timestamp
    static const uint64_t TWEPOCH = Twepoch;
    using lock_type = Lock;
public:

    // default Snowflake
    explicit SnowflakeFactory(void* ptr){}

    // k8s CIDR=-/16(ignore datacenter id)
    explicit SnowflakeFactory(uint64_t worker_id_bits = 16, uint64_t sequence_id_bits = 8){
        this->worker_id_bits = worker_id_bits;
        this->datacenter_id_bits = 0;
        this->sequence_id_bits = sequence_id_bits;
        this->initialize();
    }

    // customize snowflake
    SnowflakeFactory(uint64_t worker_id_bits, uint64_t datacenter_id_bits, uint64_t sequence_id_bits){
        this->worker_id_bits = worker_id_bits;
        this->datacenter_id_bits = datacenter_id_bits;
        this->sequence_id_bits = sequence_id_bits;
        this->initialize();
    }

    ~SnowflakeFactory() = default;

    void init_factory(uint64_t worker_id, uint64_t datacenter_id){
        std::ostringstream msg;
        if (worker_id > this->max_worker_id) {
            msg << "worker id cannot be greater than " << this->max_worker_id;
            throw std::runtime_error(msg.str());
        }
        if (this->datacenter_id_bits != 0 && datacenter_id > this->max_datacenter_id) {
            msg << "datacenter id cannot be greater than " << this->max_datacenter_id;
            throw std::runtime_error(msg.str());
        }
        if (this->datacenter_id_bits != 0){
            this->_datacenter_id = datacenter_id;
        }
        this->_worker_id = worker_id;
    }

    uint64_t next_id(){
        std::lock_guard<lock_type> lock(this->_lock);
        auto current_timestamp = uint64_t(now_timestamp());

        if (current_timestamp < this->last_timestamp){
            // Clock moved backwards, refuse to generate uid
            auto backward_time = this->last_timestamp - current_timestamp;
            std::ostringstream msg;
            msg << "Clock moved backwards, refuse to generate uid. Backward with "<< backward_time <<" seconds";
            throw std::runtime_error(msg.str());
        }

        if (this->last_timestamp == current_timestamp){
            this->_sequence_id = (this->_sequence_id + 1) & this->sequence_mask;
            // Exceed the max sequence, we wait the next mile second to generate uid
            if (this->_sequence_id == 0){
                current_timestamp = this->wait_to_next_mile_second(this->last_timestamp);
            }
        } else {
            this->_sequence_id = 0;
        }

        this->last_timestamp = current_timestamp;
        return this->gen_id(current_timestamp);
    }

protected:
    // worker id bits
    uint64_t worker_id_bits = 5 ;

    // max worker id
    uint64_t max_worker_id = (1 << worker_id_bits) - 1;

    // datacenter id bits
    uint64_t datacenter_id_bits = 5;

    // max datacenter_id_bits
    uint64_t max_datacenter_id = (1 << datacenter_id_bits) - 1;

    //  sequence id bits
    uint64_t sequence_id_bits =  12;

    // timestamp bits
    uint64_t timestamp_bits = 0;

    //  work id left shift
    uint64_t worker_id_left_shift = sequence_id_bits;

    // datacenter id shift
    uint64_t datacenter_id_left_shift = worker_id_bits + sequence_id_bits;

    // time stamp left shift
    uint64_t timestamp_left_shift = datacenter_id_bits + worker_id_bits + sequence_id_bits;

    // sequence mask
    uint64_t sequence_mask = (1 << sequence_id_bits) - 1;

    // worker id
    uint64_t _worker_id = 0;

    // datacenter id
    uint64_t _datacenter_id = 0;

    // sequence id
    uint64_t _sequence_id = 0;

    lock_type _lock;

    // last generate sfid timestamp
    uint64_t last_timestamp = 0;

    void initialize(){
        this->max_worker_id = (1 << worker_id_bits) - 1;
        this->max_datacenter_id = (1 << datacenter_id_bits) - 1;
        this->worker_id_left_shift = this->sequence_id_bits;
        this->datacenter_id_left_shift = this->worker_id_bits + this->sequence_id_bits;
        this->timestamp_left_shift = this->datacenter_id_bits + this->worker_id_bits + this->sequence_id_bits;
        this->timestamp_bits = 63 - this->worker_id_bits - this->datacenter_id_bits - this->sequence_id_bits;
        this->sequence_mask = (1 << this->sequence_id_bits) - 1;
    }

    uint64_t wait_to_next_mile_second(uint64_t _last_timestamp){
        auto timestamp = now_timestamp();
        while (timestamp <= _last_timestamp) {
            timestamp = now_timestamp();
        }
        return uint64_t(timestamp);
    }

    uint64_t gen_id(uint64_t timestamp){
        if ((timestamp - TWEPOCH) >= (uint64_t(1) << this->timestamp_bits)){
            throw std::runtime_error("over the time limit");
        }
        if (this->datacenter_id_bits == 0){
            return ((timestamp - TWEPOCH) << this->timestamp_left_shift)
                   | (this->_worker_id << this->worker_id_left_shift)
                   | this->_sequence_id;
        } else {
            return ((timestamp - TWEPOCH) << this->timestamp_left_shift)
                   | (this->_datacenter_id << this->datacenter_id_left_shift)
                   | (this->_worker_id << this->worker_id_left_shift)
                   | this->_sequence_id;
        }
    }

};


#endif //SNOWFLAKE_CPP_SNOWFLAKE_H
