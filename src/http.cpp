#include <map>
#include <vector>
#include <iostream>

#include "http.h"

void http_request::build_request(std::vector<char> &buf)
{
	std::string hdr;

	hdr += (method_str[m_method] + " ");
	hdr += (m_uri + " ");
	hdr += version_str[m_version];
	hdr += "\r\n";

	for (auto [k, v] : m_header)
	{
		hdr += (k + ":" + v + "\r\n");
	}

	/* assign */
	buf = std::vector(hdr.begin(), hdr.end());
}
