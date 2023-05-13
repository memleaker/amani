#ifndef AMANI_BUFFER_H
#define AMANI_BUFFER_H

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>

class buffer
{
public:
	buffer(size_t capacity) : m_size(0), m_capacity(capacity), m_chunked_size(0)
	{
		m_buf = std::make_unique<char[]>(capacity);
	}

	char *data() { return m_buf.get(); }
	size_t size() { return m_size; }
	size_t chunk_size() { return m_chunked_size; }
	size_t capacity() { return m_capacity; }

	void push_back(const char* buff, size_t len)
	{
		size_t copy;

		if ((m_size + len) > m_capacity)
		{
			copy = m_capacity - m_size;	
			m_chunked_size += len - copy;
		}
		else
		{
			copy = len;
		}

		memcpy(m_buf.get()+m_size, buff, copy); 
		m_size += copy;
	}

	void pop_back(size_t len)
	{
		if (len >= (m_chunked_size+m_size))
		{
			m_size = 0;
			m_chunked_size = 0;
			return;
		}

		if (len > m_chunked_size)
		{
			m_chunked_size = 0;
			m_size -= (len - m_chunked_size);
			return;
		}

		m_chunked_size -= len;
	}

private:
	size_t m_size;
	size_t m_capacity;
	size_t m_chunked_size;
	std::unique_ptr<char[]> m_buf;

public:
	class iterator
	{
	public:
		iterator(char *start, char *end) : m_pos(start), m_start(start), m_end(end) {}
		char *data(void) { return m_pos; }
		size_t residual(void) { return m_end - m_pos; }

		char operator*() { return *m_pos; }
		void operator++() { m_pos++; }
		iterator operator+(int len) { return iterator(m_pos+len, m_end); }
		void operator+=(int len) { m_pos+=len; }
		bool operator==(iterator a) { return this->data()==a.data(); }
		bool operator!=(iterator a) { return this->data()!=a.data(); }
	private:
		char  *m_pos;
		char  *m_start;
		char  *m_end;
	};

	iterator begin()
	{ return iterator(m_buf.get(), m_buf.get()+m_size); }

	iterator end()
	{ return iterator(m_buf.get()+m_size, m_buf.get()+m_size); }
};

#endif
