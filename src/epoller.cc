
#include <cerrno>
#include <sys/epoll.h>

class epoller
{
public:
    epoller() : epoll_fd(-1) {}

    int init()
    {
        epoll_fd = epoll_create(1);
	if (epoll_fd == -1)
		return -1;

	return 0;
    }

    int wait(epoll_event* evs, int events)
    {
		int ret;

again:
		ret = epoll_wait(epoll_fd, evs, events, -1);
		if (ret == -1)
		{
			if (errno == EINTR)
				goto again;

			/* fatal error, need exit */
			return -1;
		}

		return ret;
    }

private:
    int epoll_fd;
};
