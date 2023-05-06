#include "url.h"

void url::parse(std::string url)
{
	size_t pos;
	int po;
	std::string addr;
	char ip[16];

	// "protocol://"
	if ((pos = url.find("://")) != url.npos)
	{
		proto = url.substr(0, pos);
		if (proto == "http")
		{
			port = 80;
		}
		else if (proto == "https")
		{
			port = 443;
		}
		else
		{
			std::cerr << "unknown protocol " << proto << std::endl;
			std::exit(1);	
		}

		url = url.substr(pos+3);
	}

	// uri
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

	// domain
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
		std::cerr << "unresolvable" << std::endl;
		std::exit(1);
	}

	if (!host->h_addr_list[0]) {
		std::cerr << "unresolvable" << std::endl;
		std::exit(1);
	}

	inet_ntop(AF_INET, (in_addr*)host->h_addr_list[0], ip, sizeof(ip));
	ipaddr = ip;
}