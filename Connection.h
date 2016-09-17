#ifndef __CONNECTION_H_
#define __CONNECTION_H_

#include <ev.h>
#include <boost/enable_shared_from_this.hpp>
#include "Commondef.h"


class Connection : public boost::enable_shared_from_this<Connection>
{
public:
	Connection(int fd, struct sockaddr_in addr,Server* server,EventLoopThread* eventLoopThread);
	~Connection();

	void InitConnectin(struct ev_loop* loop);

	bool ReadAndDisptchMessage();

private:

	int    m_clientFd;
	struct sockaddr_in m_clientAddr;

	struct ev_io  m_ev_read;
	struct ev_io  m_ev_write;
	struct ev_loop*  m_eventLoop;

	Server* m_server;
	EventLoopThread* m_eventLoopThread;

};

#endif
