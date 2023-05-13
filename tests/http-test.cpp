#include <string>
#include <iostream>

#include "buffer.h"
#include "http.h"

int main(void)
{
	std::string rsp = "HTTP/1.1 200 OK\r\nContent-Length:10\r\naaaa:eaaaaa\r\na:e\r\n\r\ndddddddddd";//eee:ddd\r\n";
	std::string rsp2 = "HTTP/1.1 200 OK\r\nTransfer-Encoding:chunked\r\naaaa:eaaaaa\r\na:e\r\n\r\n1\r\na2\r\ndd0\r\n";//eee:ddd\r\n";

	buffer buf(1024);
	buffer buf2(1024);

	buf.push_back(rsp.c_str(), rsp.size());
	buf2.push_back(rsp2.c_str(), rsp2.size());

	http_response resp;
	http_response resp2;

	std::cout << resp.parse(buf) << std::endl;
	std::cout << resp2.parse(buf2) << std::endl;
}