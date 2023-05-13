#ifndef AMANI_UTILS_H
#define AMANI_UTILS_H

#include <unistd.h>

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
};

#endif
