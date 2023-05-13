#ifndef AMANI_HTTP_H
#define AMANI_HTTP_H

#include <string>
#include <vector>
#include <map>

#include "buffer.h"
#include "argument.h"

static std::map<http_version, std::string> version_str = 
{
	{HTTP10, "HTTP1.0"},
	{HTTP11, "HTTP1.1"},
};

static std::map<http_method, std::string> method_str = 
{
	{GET, "GET"},
	{POST, "POST"},
};

class http_request
{
public:
	http_request() : m_uri("/"), m_method(GET), m_version(HTTP11)
	{
		m_header["User-Agent"] = "amaniv1.0";	
	}

    void set_uri(const std::string &uri) { m_uri = uri; }
    void set_method(http_method meth) { m_method = meth; }
    void set_version(http_version version) { m_version = version; }
    void set_header(const std::string &key, std::string &val) {m_header[key] = val;}
    void build_request(std::vector<char> &buf);

private:
    // request line
    std::string m_uri;
    http_method m_method;
    http_version m_version;

    // request header
    std::map<std::string, std::string> m_header;
};

class http_response
{
public:
    int parse(buffer& buf);
    int parse_version(buffer::iterator& it, buffer& buf);
    int parse_status(buffer::iterator& it, buffer& buf);
    int parse_reason(buffer::iterator& it, buffer& buf);
    int parse_header(buffer::iterator& it, buffer& buf);
    int parse_body(buffer::iterator& it, buffer& buf);
    int parse_chunked_body(buffer::iterator& it, buffer& buf);
    int parse_block_len(buffer::iterator& it, buffer& buf);
    int parse_block_data(int size, buffer::iterator& it, buffer& buf);

public:
    // response line
    int status_code;
    std::string version;
    std::string reason;

    std::map<std::string, std::string> headers;
};

#endif
