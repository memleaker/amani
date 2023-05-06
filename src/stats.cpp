#include <ctime>
#include <iostream>

#include <unistd.h>

#include "stats.h"

void print_status(stats& st, unsigned int seconds, unsigned int livetime)
{
    time_t start, curr;
    unsigned int cost {0};

    std::cout << "Statistic about " << st.desturl << "(" << st.destip << ")" << std::endl;
    std::cout << "\tInterval" << "\t\t" << "Request" << "\t\t" << "Response" << std::endl;

    for (curr = (start = std::time(NULL)); (curr - start) < livetime;
    	 curr = std::time(NULL))
    {
        stats last(st);

	    ::sleep(seconds);

        // print
        std::cout << "\t"   << cost << "-" << cost+seconds << " sec" \
            << "\t\t" << st.resq-last.resq << " times" \
            << "\t\t" << st.resp-last.resp << " times" << std::endl;
        }
}
