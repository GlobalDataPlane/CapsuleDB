// ----------------------------------------
// HotCalls
// Copyright 2017 The Regents of the University of Michigan
// Ofir Weisse, Valeria Bertacco and Todd Austin

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ---------------------------------------------

//Author: Ofir Weisse, www.OfirWeisse.com, email: oweisse (at) umich (dot) edu
//Based on ISCA 2017 "HotCalls" paper.
//Link to the paper can be found at http://www.ofirweisse.com/previous_work.html
//If you make nay use of this code for academic purpose, please cite the paper.

#ifndef __COMMON_H
#define __COMMON_H

#define NET_CLIENT_IP "35.247.29.237"
#define NET_SEED_SERVER_IP "34.83.138.211"
#define NET_SYNC_SERVER_IP "34.83.138.211"
#define NET_KEY_DIST_SERVER_IP "34.83.138.211"

// // ip of this machine
// #define NET_CLIENT_IP "localhost"
// // ip of seed server(router)
// #define NET_SEED_SERVER_IP "localhost"
// // ip of sync coordinator
// #define NET_SYNC_SERVER_IP "localhost"
// // ip of key distribution server
// #define NET_KEY_DIST_SERVER_IP "localhost"

// Key for coordinator Request To Send(RTS)
#define COORDINATOR_RTS_TYPE "PARANOID_RTS"
// Key for latest sync packet
#define COORDINATOR_SYNC_TYPE "PARANOID_SYNC"
// Key for latest EOE
#define COORDINATOR_EOE_TYPE "PARANOID_EOE"

#define START_CLIENT_ID 2 // >=2 ; TOTAL_THREADS - START_CLIENT_ID = Num of clients on this node
#define TOTAL_THREADS 3
#define EPOCH_TIME 2
#define PERFORMANCE_MEASUREMENT_NUM_REPEATS 10
#define NUM_CRYPTO_ACTORS 2
#define BATCH_SIZE 2
// #define RUN_BOTH_CLIENT_AND_SERVER true
#define NET_CLIENT_BASE_PORT 5555
#define NET_SYNC_SERVER_PORT 5556
#define NET_SERVER_JOIN_PORT 6666
#define NET_SERVER_MCAST_PORT 6667
#define NET_KEY_DIST_SERVER_PORT 3001

#define BENCHMARK_MODE false
// #define LOGI LOG_IF(INFO, !BENCHMARK_MODE)
// #define LOGD LOG_IF(INFO, BENCHMARK_MODE) << get_current_time() << " "
#if BENCHMARK_MODE
#include "benchmark.h"
//#define M_BENCHMARK_CODE void KVSClient::benchmark(){}
#define M_BENCHMARK_CODE M_BENCHMARK_HERE
#else
#define M_BENCHMARK_CODE \
    void benchmark() {}
#endif

enum OCALL_ID
{
    OCALL_PUT,
};

enum ECALL_ID
{
    ECALL_PUT,
    ECALL_RUN,
};

typedef struct
{
    uint64_t *cyclesCount;
    uint64_t counter;
    void *data;
    OCALL_ID ocall_id;
} OcallParams;

typedef struct
{
    void *data;
    ECALL_ID ecall_id;
} EcallParams;

#endif
