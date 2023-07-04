#ifndef AMANI_UTILS_H
#define AMANI_UTILS_H

#include <cstdint>
#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/time.h>

class utils
{
public:
    static long cpu_num(long min)
    {
        long cpus;

        cpus = sysconf(_SC_NPROCESSORS_ONLN);
        return (cpus > min) ? cpus : min;
    }

    static void set_limit()
    {
        
    }

	static uint64_t rtt(struct timeval start, struct timeval end)
	{
		uint64_t rtt;

		rtt = (end.tv_sec - start.tv_sec) * 1000 * 1000;
		rtt += end.tv_usec - start.tv_usec;

		return rtt;
	}

    static double average_rtt(std::vector<uint64_t> &rtts, uint64_t start, uint64_t end)
    {
        uint64_t avrtt = 0;

        for (uint64_t i = start; i < end; i++)
        {
            avrtt+=rtts[i];
        }

        if (avrtt == 0)
            return 0;

        return ((double)avrtt / 1000) / (double)(end-start);
    }
};

#endif
