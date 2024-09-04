#ifndef AMANI_POLLER_H
#define AMANI_POLLER_H

#include <cstdint>

#include "netio_task.h"

class poller
{
public:
	virtual int ioevent_add(netio_task*, uint32_t) = 0;
	virtual int ioevent_del(int) = 0;
	virtual int ioevent_handle(void) = 0;
	/* 
	 * 虽然析构函数可以是纯虚函数, 但也要提供实现
	 * 因此直接写成虚函数, 而非纯虚函数
	 */
    virtual ~poller() {};
};

#endif