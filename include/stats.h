#ifndef AMANI_STATS_H
#define AMANI_STATS_H

#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <sys/time.h>

class stats
{
    uint64_t resq;
    uint64_t resp;

    struct timeval start;
    struct timeval end;

    float aver_qps;  // 评价每秒请求数量(不算错误的数量，服务端返回错误的时候不算)
    float aver_rtt;
    std::vector<float> tot_rtts;

    std::string destip;
    std::string desturl;

    // 状态码以及数量
    std::map<int, uint64_t> status_codes;
};

#endif