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
    argument() : https(false), clients(1), time(10), http_version(HTTP11), meth(GET) {}

    void parse(int argc, char **argv);

    bool https;
	int clients;
	int time;
	enum http_version http_version;
    enum http_method meth;
	url urlinfo;
	std::string urlstr;
    std::string reqfile;

	std::map<std::string, std::string> req_header;
};

#endif