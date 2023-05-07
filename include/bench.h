#ifndef AMANI_BENCH_H
#define AMANI_BENCH_H

#include <iostream>
#include <vector>
#include <cstddef>

#include "http.h"
#include "copool/netio.h"

netio_task http10_bench(std::vector<char>& req, uint32_t ipaddr, uint16_t port)
{
	int sock, ret;
	sockaddr_in addr;
	netio_task task;
	http_response resp;
	ssize_t nleft, nwrite, n;

	sock = amani_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1)
	{
		std::cerr << "createa socket failed: " << std::strerror(errno) << std::endl;
		co_return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ipaddr;
	addr.sin_port = htons(port);

	for (;;)
	{
		/* connect */
		ret = co_await async_connect(sock, (sockaddr*)&addr, sizeof(addr));
		if (ret == -1)
		{
			std::cerr << "connect failed: " << std::strerror(errno) << std::endl;
			co_return -1;
		}

		/* send data */
		for (nleft = req.size(), nwrite = 0; nleft > 0; )
		{
			n = co_await async_write(sock, req.data()+nwrite, nleft);
			if (n == -1)
			{
				std::cerr << "send request failed: " << std::strerror(errno) << std::endl;
				co_return -1;
			}

			nwrite += n;
			nleft  -= n;
		}

		// recv_response(resp, sock);
		sleep(1);
		// close(sock);
	}
}

#endif