#ifndef AMANI_NETIO_H
#define AMANI_NETIO_H

#include <cstddef>
#include <cstdint>
#include <cerrno>

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "copool.h"

int amani_socket(int domain, int type, int protocol)
{
	return socket(domain, type | SOCK_NONBLOCK, protocol);
}

int amani_listen(int fd, uint32_t ipaddr, uint16_t port)
{
	int ret;
	sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	addr.sin_addr.s_addr = ipaddr;

	ret = bind(fd, (const sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
		return ret;

	ret = listen(fd, 128);
	if (ret == -1)
		return ret;

	return 0;
}

class async_accept {
public:
	async_accept(int fd, sockaddr* addr, socklen_t *addrlen) : m_fd(fd), m_addr(addr), m_addrlen(addrlen) {}

    bool await_ready()
	{
		int ret;

		ret = accept4(m_fd, m_addr, m_addrlen, SOCK_NONBLOCK);		
		if (ret == -1 && errno == EAGAIN)
			return false;  // suspend
		std::cout << "ready" << std::endl;

		return true;
	}

    void await_suspend(std::coroutine_handle<netio_task::promise_type> handle)
	{
		handle.promise().fd = m_fd;
		handle.promise().flags = EPOLLIN;
		handle.promise().need_block = 1;
	}

    ssize_t await_resume()
	{
		int ret;

		std::cout << "resume" << std::endl;

		for (;;)
		{
			ret = accept4(m_fd, m_addr, m_addrlen, SOCK_NONBLOCK);		
			if (ret == -1)
			{
				if (errno == EINTR || errno == EAGAIN)
					continue;
			}

			return ret;
		}
	}

private:
	int     m_fd;
	sockaddr   *m_addr;
	socklen_t  *m_addrlen;
};


class async_read {
public:
	async_read(int fd, void *buf, size_t len) : m_fd(fd), m_buf(buf), m_len(len) {}

    bool await_ready()
	{
		// sys_read 即真正的read系统调用
		m_nbytes = read(m_fd, m_buf, m_len);
		if (m_nbytes == -1 && errno == EAGAIN)
			return false;  // suspend

		return true;
	}

    void await_suspend(std::coroutine_handle<netio_task::promise_type> handle)
	{
		handle.promise().fd = m_fd;
		handle.promise().flags = EPOLLIN;
		handle.promise().need_block = 1;
	}

    ssize_t await_resume()
	{
		for (;;)
		{
			m_nbytes = read(m_fd, m_buf, m_len);
			if (m_nbytes == -1)
			{
				if (errno == EINTR || errno == EAGAIN)
					continue;
			}

			return m_nbytes;
		}
	}

private:
	int     m_fd;
	void *  m_buf;
	size_t  m_len;
	ssize_t m_nbytes;
};










#endif