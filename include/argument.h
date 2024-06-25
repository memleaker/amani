#ifndef AMANI_ARGUMENT_H
#define AMANI_ARGUMENT_H

#include <string>
#include <map>

#include <getopt.h>

#include "url.h"

enum http_version {HTTP10, HTTP11};
enum http_method  {GET, POST};

class argument
{
public:
	/* DEFAULT Arg: one clients, 10 seconds, HTTP1.1, GET request. */
    argument() : clients(1), time(10), http_version(HTTP11), meth(GET) {}

    void parse(int argc, char **argv);

public:
	int clients;
	int time;
	enum http_version http_version;
    enum http_method meth;

public:
	std::string urlstr; /* Argument: URL String */
	url urlinfo;        /* URL Parse */
};

#endif