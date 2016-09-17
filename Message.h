#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include <string.h>
#include <boost/shared_ptr.hpp>

struct  Message
{  
	struct Header
	{
		char magic[4];   // "fm"
		int  msgid;
		int  length;     // length of the message body
	};
	Header header;
	unsigned char body[1];

	static Message* Allocate(int id, int len);
	static Message* Allocate(int id, int len, const void* body);
	static Message* Allocate(int id, const char* body);
	static Message* Allocate(int id, const std::string& body);
	static void Free(Message* msg);
	static bool Verify(Message* msg);
};

#endif