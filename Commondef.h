#ifndef __COMMONDEF_H_
#define __COMMONDEF_H_

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <CommonSDK/CommonSDK.h>

#include "Message.h"

#define EVENT_LOOP_THREAD_MAX 4

class Server;
class EventLoopThread;
class Connection;

typedef boost::shared_ptr<Connection> ConnectionPtr;
typedef std::vector<ConnectionPtr> ConnectList;
typedef std::list<Message*> MessageList;
typedef boost::shared_ptr<boost::thread> ThreadPtr;

#endif
