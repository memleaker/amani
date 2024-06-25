#include "url.h"

#include <map>
#include <string>

static std::map<std::string, uint16_t> proto_map = {
	{"http", 80},
	{"https", 443},
};

/*
 * Parse URL: 
 *  http://[host]:[port]/[uri], port default 80
 *  https://[host]:[port]/[uri], port default 443
*/
void url::parse(std::string url)
{
	size_t pos;
	int po;
	std::string addr;
	char ip[16];

	/* 1. Parse Protocol  Default: Http */
	if ((pos = url.find("://")) != url.npos)
	{
		proto = url.substr(0, pos);
		if (proto_map.find(proto) != proto_map.end())
		{
			port = proto_map[proto];
		}
		else
		{
			std::cerr << "Invalid URL: only support http or https " << proto << std::endl;
			std::exit(1);
		}

		url = url.substr(pos+3);
	}

	/* 2. Get HTTP REQUEST URI */
	if ((pos = url.find("/")) == url.npos)
	{
		uri = "/";
		addr = url;
	}
	else
	{
		uri = url.substr(pos);
		addr = url.substr(0, pos);
	}

	/* 3. Parse Domanin name, Get IP and PORT
	 *    Default: 127.0.0.1:80
	 */
	if ((pos = addr.find(":")) != addr.npos)
	{
		domain_name = addr.substr(0, pos);

		po = std::atoi(addr.substr(pos+1).c_str());
		if (po <= 0 || po > 65535)
		{
			std::cerr << "invalid port " << addr.substr(pos) << std::endl;
			std::exit(1);
		}

		port = po;
	}
	else
	{
		domain_name = addr;
	}

	hostent *host = gethostbyname(domain_name.c_str());
	if (!host) {
		std::cerr << "domain name: " << domain_name << "unresolvable" << std::endl;
		std::exit(1);
	}

	if (!host->h_addr_list[0]) {
		std::cerr << "domain name: " << domain_name << "unresolvable" << std::endl;
		std::exit(1);
	}

	inet_ntop(AF_INET, (in_addr*)host->h_addr_list[0], ip, sizeof(ip));
	ipaddr = ip;
}