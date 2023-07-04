#include <ctime>
#include <cstdio>
#include <cstdint>
#include <iostream>

#include <unistd.h>

#include "stats.h"
#include "utils.h"

void stats::print_status(int time, std::string desturl, std::string destip)
{
    double rtt;
    static uint64_t lastreq = 0, lastresp = 0, lastok = 0;
    uint64_t curreq, curresp, ok;
    char buf[2048], interval[32];

    std::cout << "Statistic about " << desturl << "(" << destip << ")" << std::endl;
    snprintf(buf, sizeof(buf), "%-16s%-16s%-16s%-16s%-16s", 
                  "Interval", "Request", "Response", "Rtt", "200OK");
    std::cout << buf << std::endl;

    for (int t = 0; t < time; t++)
    {
        curreq  = req;
        curresp = resp;
        ok      = status_codes[200];
        rtt     = utils::average_rtt(rtts, lastresp, curresp);

        /* print */
        snprintf(interval, sizeof(interval), "%llu-%llu sec", t, t+1);
        std::cout << t << "-" << t+1 << " sec" << "\t\t"
                  << curreq - lastreq << " qps" << "\t\t"
                  << curresp - lastresp << " pps" << "\t\t"
                  << rtt << " ms" << "\t\t"
                  << ok - lastok << std::endl;

        /* update */
        lastreq  = curreq;
        lastresp = curresp;
        lastok   = ok;

        sleep(1);
    }
}
