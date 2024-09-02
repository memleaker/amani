#ifndef AMANI_POLLER_H
#define AMANI_POLLER_H

#include <cstdint>

#include "netio_task.h"

struct poll_data
{
	int fd;
	void *run_state;
};

class poller
{
public:
	virtual int ioevent_add(netio_task*, uint32_t) = 0;
	virtual int ioevent_del(int) = 0;
	virtual int ioevent_handle(void) = 0;
    virtual ~poller() = 0;
};

#endif