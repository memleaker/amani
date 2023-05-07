#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include "copool/copool.h"
#include "copool/netio.h"

#define PORT 8888

netio_task echo_io(int fd)
{
	ssize_t n;
	char buf[1024];

	for (;;)
	{
		n = co_await async_read(fd, buf, sizeof(buf));
        if (n == -1)
        {
            std::cerr << "read error: " << std::strerror(errno) << std::endl;
            close(fd);
            co_return;
        }
        else if (n == 0)
        {
            std::cerr << "connection closed by peer" << std::endl;
            close(fd);
            co_return;
        }

		buf[n] = '\0';
		std::cout << "recv: " << buf << std::endl;

        n = co_await async_write(fd, buf, n);
        if (n == -1)
        {
            std::cerr << "write error: " << std::strerror(errno) << std::endl;
            close(fd);
            co_return;
        }
	}
}

netio_task echo_server(netco_pool& pool)
{
	int fd, ret, connfd;
	char ip[16];
	sockaddr_in addr;
	socklen_t addrlen;

    /* socket */
	fd = amani_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1)
    {
        std::cerr << "create socket failed: " << std::strerror(errno) << std::endl;
        std::exit(1);
    }

    /* bind and listen */
	ret = amani_listen(fd, INADDR_ANY, PORT);
    if (ret == -1)
    {
        std::cerr << "bind or listen failed: " << std::strerror(errno) << std::endl;
        std::exit(1);
    }

    std::cout << "listening at 0.0.0.0:" << PORT << std::endl;

	for (;;)
	{
        /* accept */
		connfd = co_await async_accept(fd, (sockaddr*)&addr, &addrlen);

		std::cout << "accept conn from: " << inet_ntop(AF_INET, &addr.sin_addr.s_addr, ip, sizeof(ip)) \
                  <<  ":" << ntohs(addr.sin_port) << std::endl;

        /* echo */
		pool.submit(echo_io, connfd);
	}
}

int main(void)
{
	netco_pool pool(4);
	pool.init();

    /* running */
	pool.submit(echo_server, pool);
	pool.run();

    /* wait */
    for (;;) sleep(1);

	/* stop */
	pool.shutdown();
}