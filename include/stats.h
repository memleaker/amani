#ifndef AMANI_STATS_H
#define AMANI_STATS_H

#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <sys/time.h>

#include <mutex>

class stats
{
public:
	stats() : req(0),resp(0) {}

private:
    uint64_t req;
    uint64_t resp;

	std::mutex stat_mutex;
    std::vector<uint64_t> rtts;
    std::map<int, uint64_t> status_codes;

public:
	void inc_request(void)
	{
		std::lock_guard<std::mutex> lock(stat_mutex);
		req++;	
	}

	void inc_response(void)
	{
		std::lock_guard<std::mutex> lock(stat_mutex);
		resp++;
	}

	void inc_status(int code)
	{
		std::lock_guard<std::mutex> lock(stat_mutex);
		status_codes[code]++;
	}

	void set_rtt(uint64_t rtt)
	{
		std::lock_guard<std::mutex> lock(stat_mutex);
		rtts.push_back(rtt);
	}

	void print_status(int time, std::string desturl, std::string destip);
};

#endif
