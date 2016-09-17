#ifndef __EVENT_LOOP_THREAD_H_
#define __EVENT_LOOP_THREAD_H_

#include "ev.h"
#include "Commondef.h"

class Server;

class EventLoopThread
{
public:
	EventLoopThread(int listenfd, int readfd, Server* server);

	~EventLoopThread();

	void Initialize();

	void NotifyEventArrived();

	static void WorkThread(void* arg);

	int GetListenFdCount();

	void IncreaseFdCount();

	void DecreaseFdCount();

private:
	boost::mutex m_mutex;

	ThreadPtr m_thread;

	int m_listenfd;

	int m_readfd;

	Server* m_server;

	struct ev_loop* m_eventLoop;

	int m_fdCount;
};


#endif
