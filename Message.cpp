#include "Message.h"

Message* Message::Allocate(int id, int len)
{
	int total_len = sizeof(Message)+len;
	Message* msg = (Message*)malloc(total_len);
	memset(msg, 0, total_len);
	msg->header.magic[0] = 'f';
	msg->header.magic[1] = 'm';
	msg->header.msgid = id;
	msg->header.length = len;
	return msg;
}

Message* Message::Allocate(int id, int len, const void* body)
{
	Message* msg = Allocate(id, len);
	memcpy(&(msg->body[0]), body, len);
	return msg;
}

Message* Message::Allocate(int id, const char* body)
{
	int len = int(strlen(body));
	Message* msg = Allocate(id, len);
	strcpy((char*)&(msg->body[0]), body);
	return msg;
}

Message* Message::Allocate(int id, const std::string& body)
{
	int len = int(body.length());
	Message* msg = Allocate(id, len);
	strcpy((char*)&(msg->body[0]), body.c_str());
	return msg;
}

void Message::Free(Message* msg)
{
	free((void*)msg);
}

bool Message::Verify(Message* msg)
{
	return msg->header.magic[0] == 'f' && msg->header.magic[1] == 'm';
}
