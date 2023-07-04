#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <istream>
#include <sstream>

#include <unistd.h>
#include "http.h"

void http_request::build_request(std::vector<char> &buf)
{
	std::string hdr;

	hdr += (method_str[m_method] + " ");
	hdr += (m_uri + " ");
	hdr += version_str[m_version];
	hdr += "\r\n";

	hdr += ("Connection: "+ conn_str[m_version] + "\r\n");

	for (auto [k, v] : m_header)
	{
		hdr += (k + ":" + " " + v + "\r\n");
	}

	hdr += "\r\n";

	/* assign */
	buf = std::vector(hdr.begin(), hdr.end());
}

/*
 * Return 0  on Success.
 * Return -1 : buf is not a complete response.
 * Return -2 : invalid http response. 
*/
int http_response::parse(buffer& buf)
{
	int result;
	auto p = buf.begin();

	/* request line */
	result = parse_version(p, buf);
	if (result < 0)
		return result;
	
	result = parse_status(p, buf);
	if (result < 0)
		return result;
	
	result = parse_reason(p, buf);
	if (result < 0)
		return result;
	
	/* request header */
	result = parse_header(p, buf);
	if (result < 0)
		return result;

	/* body */
	result = parse_body(p, buf);
	if (result < 0)
		return result;

	return 0;
}

int http_response::parse_version(buffer::iterator& it, buffer& buf)
{
	auto start = it;

	while (it != buf.end())
	{
		switch (*it)
		{
		case ' ':
			if (start == it)
				return -2;

			version = std::string(start.data(),it.data());
			if (version != "HTTP/1.0" && version != "HTTP/1.1")
				return -2;

			++it;
			return 0;
		default:
			++it;
			break;
		}
	}

	return -1;
}

int http_response::parse_status(buffer::iterator& it, buffer& buf)
{
	auto start = it;

	while (it != buf.end())
	{
		switch (*it)
		{
		case ' ':
		{
			if (start == it)
				return -2;

			std::istringstream sc(std::string(start.data(),it.data()));
			sc >> status_code;
			++it;
			return 0;
		}
		default:
			++it;
			break;
		}
	}

	return -1;
}

int http_response::parse_reason(buffer::iterator& it, buffer& buf)
{
	auto start = it;

	while (it != buf.end())
	{
		switch (*it)
		{
		case '\r':
			if (*(it+1) != '\n')
				return -2;

			if (start == it)
				return -2;

			reason = std::string(start.data(), it.data());
			it += 2;
			return true;
		default:
			++it;
			break;
		}
	}

	return 0;
}

std::tuple<int, std::string> parse_hdrkey(buffer::iterator& it, buffer& buf)
{
	auto start = it;
	std::string key;

	while (it != buf.end())
	{
		switch (*it)
		{
		case ':':
			if (start == it)
				return std::make_tuple(-2, "");

			key = std::string(start.data(), it.data());
			++it;
			return std::make_tuple(0, key);
		default:
			++it;
			break;
		}
	}

	return std::make_tuple(-1, "");
}

std::tuple<int, std::string> parse_hdrval(buffer::iterator& it, buffer& buf)
{
	auto start = it;
	std::string val(1024, '0');

	while (it != buf.end())
	{
		switch (*it)
		{
		case '\r':
			if (*(it+1) != '\n')
				return std::make_tuple(-2, "");;

			if (start == it)
				return std::make_tuple(-2, "");;

			val = std::string(start.data(), it.data());
			it += 2;
			return std::make_tuple(0, val);;
		default:
			++it;
			break;
		}
	}

	return std::make_tuple(-1, "");;
}

int http_response::parse_header(buffer::iterator& it, buffer& buf)
{
	int ret;
	std::string key, val;

	for (;;)
	{
		std::tie(ret, key) = parse_hdrkey(it, buf);
		if (ret < 0)
			return ret;

		std::tie(ret, val) = parse_hdrval(it, buf);
		if (ret < 0)
			return ret;

		/* save header */
		headers[key] = val;

		/* check request body */		
		if (it == buf.end())
		{
			return 0;
		}
		
		if (it != buf.end() && (it+1) != buf.end() && 
			*it == '\r' && *(it+1) == '\n')
		{
			it += 2;
			return 0;
		}
	}

	return -1;
}

int http_response::parse_block_data(int size, buffer::iterator& it, buffer& buf)
{
	if (it.residual() < (size_t)size)
		return -1;
	
	it += size;

	return 0;
}

int http_response::parse_block_len(buffer::iterator& it, buffer& buf)
{
	int len;
	auto start = it;

	while (it != buf.end())
	{
		if (isdigit(*it))
		{
			++it;
			continue;
		}

		if (*it == '\r')
		{
			if ((it+1) == buf.end())
				return -1;
			
			if (*(it+1) != '\n')
				return -2;

			if (start == it)
				return -2;
			
			std::istringstream ss(std::string(start.data(), it.data()));
			ss >> len;

			if (len < 0)
				return -2;

			it += 2;
			return len;
		}
		
		return -2;
	}

	return -1;
}

int http_response::parse_chunked_body(buffer::iterator& it, buffer& buf)
{
	int n;

	for (;;)
	{
		n = parse_block_len(it, buf);
		if (n < 0)
			return n;
		else if (n == 0)
			return 0;

		n = parse_block_data(n, it, buf);
		if (n < 0)
			return n;
	}
}

int http_response::parse_body(buffer::iterator& it, buffer& buf)
{
	size_t length;

	auto itt = headers.find("Content-Length");
	if (itt != headers.end())
	{
		std::istringstream ss(itt->second);
		ss >> length;

		if ((buf.chunk_size() + it.residual()) == length)
			return 0;
		else
			return -1;
	}

	if (headers["Transfer-Encoding"] == "chunked")
	{
		/* cannot parse. it's greater than buffer size */
		if (buf.chunk_size() > 0)
			return -2;

		return parse_chunked_body(it, buf);
	}

	return -2;
}
