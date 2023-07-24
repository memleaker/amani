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
    char buf[2048], interval[32], qps[32], pps[32], rttms[32];

    std::cout << "Statistic about " << desturl << "(" << destip << ")" << std::endl;
    snprintf(buf, sizeof(buf), "%-18s%-18s%-18s%-18s%-18s", 
                  "Interval", "Request", "Response", "Rtt", "200OK");
    std::cout << buf << std::endl;

    for (int t = 0; t < time; t++)
    {
        sleep(1);

        curreq  = req;
        curresp = resp;
        ok      = status_codes[200];
        rtt     = utils::average_rtt(rtts, lastresp, curresp);

        /* print */
        snprintf(interval, sizeof(interval), "%d-%d sec", t, t+1);
		snprintf(qps, sizeof(qps), "%lu qps", curreq - lastreq);
		snprintf(pps, sizeof(pps), "%lu pps", curresp - lastresp);
		snprintf(rttms, sizeof(rttms), "%lf ms", rtt);
		printf("%-18s%-18s%-18s%-18s%-18lu\n", interval, qps, pps, rttms, ok-lastok);
        //std::cout << t << "-" << t+1 << " sec" << "\t\t"
        //          << curreq - lastreq << " qps" << "\t\t"
        //          << curresp - lastresp << " pps" << "\t\t"
        //          << rtt << " ms" << "\t\t"
        //          << ok - lastok << std::endl;

        /* update */
        lastreq  = curreq;
        lastresp = curresp;
        lastok   = ok;
    }

	/* total */
	std::cout << "Total Request: " << req << std::endl;
	std::cout << "Total Response: " << resp << std::endl;
	std::cout << "Status Code: " << std::endl;
	for (auto [k, v] : status_codes)
	{
		std::cout << "Code: " << k << "\t" << "Times: " << v << std::endl;
	}
}
