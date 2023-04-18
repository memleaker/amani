#ifndef AMANI_HTTP_H
#define AMANI_HTTP_H

#include <string>
#include <map>

class http_request
{
public:
    void set_uri(const std::string &uri) { m_uri = uri; }
    void set_method(const std::string &method) { m_method = method; }
    void set_version(const std::string &version) { m_version = version; }
    void set_header(const std::string &key, std::string &val) {m_header[key] = val;}
    void set_body(const std::string &body) { m_body = body; }
    std::string to_string(const http_request& req);

public:
    // request line
    std::string m_uri;
    std::string m_method;
    std::string m_version;

    // request header
    std::map<std::string, std::string> m_header;

    // request body
    std::string m_body;
};

class http_response
{
public:
    http_response& parse(const std::string& resp);

public:
    // response line
    int status_code;
    std::string version;
    std::string reason;
};

#endif