#ifndef AMANI_URL_H
#define AMANI_URL_H

#include <string>
#include <iostream>

#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

class url
{
public:
	url() : port(0), proto("http"), ipaddr("0.0.0.0") {}
	void parse(std::string url);

public:
	uint16_t    port;
	std::string proto;
	std::string ipaddr;
	std::string domain_name;
	std::string uri;
};

#endif