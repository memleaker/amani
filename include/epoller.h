#ifndef AMANI_EPOLLER_H
#define AMANI_EPOLLER_H

#include <iostream>

#include <cstddef>
#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <unistd.h>
#include <sys/epoll.h>

class epoller
{
public:
    epoller() : epoll_fd(-1) {}

    void init()
    {
        epoll_fd = epoll_create(1);
		if (epoll_fd == -1)
		{
			std::cerr << "error: epoll create " << std::strerror(errno) << std::endl;
			std::exit(1);
		}
    }

    int add_fd(int fd, uint32_t events)
    {
		epoll_event ev;

		// 事件只触发一次，不然task_queue里面会push好几个任务啦
		ev.events = events | EPOLLONESHOT;
		ev.data.fd = fd;

    	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev))
		{
			std::cerr << "error: epoll add fd " << std::strerror(errno) << std::endl;
			std::exit(1);
		}
		
		return 0;
    }

	// 使用 EPOLLONESHOT依然需要删除，因为add了就存在了，使用了oneshot也不会删除
	int del_fd(int fd)
	{
		return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	}

    int wait(epoll_event* evs, int events)
    {
		int ret;

again:
		ret = epoll_wait(epoll_fd, evs, events, 1);
		if (ret == -1)
		{
			if (errno == EINTR)
				goto again;

			/* fatal error, need exit */
			std::cerr << "epoll_wait " << std::strerror(errno) << std::endl;
			std::exit(1);
		}

		return ret;
    }

	~epoller() { close(epoll_fd); }

private:
    int epoll_fd;
};

#endif
