#include "EventLoopThread.h"
#include "Connection.h"
#include "Server.h"

void NotifyDataArrived(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	//std::cout<<"NotifyDataArrived"<<std::endl;
	EventLoopThread* eventLoop = (EventLoopThread*)watcher->data;
	eventLoop->NotifyEventArrived();
}

EventLoopThread::EventLoopThread(int listenfd, int readfd, Server* server)
	:m_listenfd(listenfd), m_readfd(readfd), m_server(server),m_eventLoop(NULL),m_fdCount(0)
{

}

EventLoopThread::~EventLoopThread()
{
	if (m_thread)
	{
		m_thread->join();
	}
	close(m_readfd);
	boost::unique_lock<boost::mutex> lock(m_mutex);
	m_fdCount = 0;

	std::cout<<"EventLoopThread::~EventLoopThread"<<std::endl;
}

void EventLoopThread::Initialize()
{
	m_thread = ThreadPtr(new boost::thread(boost::bind(EventLoopThread::WorkThread, this)));
}

void EventLoopThread::WorkThread(void* arg)
{
	EventLoopThread* pThis = (EventLoopThread*)arg;

	struct ev_loop* eventLoop = ev_loop_new(EVBACKEND_EPOLL);
	pThis->m_eventLoop = eventLoop;

	struct ev_io notify_watcher;
	notify_watcher.data = pThis;
	ev_io_init (&notify_watcher, NotifyDataArrived, pThis->m_readfd, EV_READ);
	ev_io_start (eventLoop, &notify_watcher);

	ev_loop(eventLoop,0);
}

void EventLoopThread::NotifyEventArrived()
{
	char buf[32] = {0};
	if (read(m_readfd, buf, 32) <= 0)
	{
		return;
	}
	if (strcmp(buf, "New Connection") == 0)
	{
		int  connfd;
		struct sockaddr_in cliaddr;
		socklen_t cliaddrlen = sizeof(cliaddr);

		if ((connfd = accept(m_listenfd,(struct sockaddr*)&cliaddr,&cliaddrlen)) == -1)
		{
			//std::cout<<"Accept Error"<<std::endl;
			return;
		}
		Server::MakeNonBlocking(connfd);

		std::cout<<fm::Time::Now().FormatString()<<":accept a new client:"<<inet_ntoa(cliaddr.sin_addr)<<":"<<cliaddr.sin_port<<std::endl;
		ConnectionPtr conn = ConnectionPtr(new Connection( connfd,cliaddr,m_server,this ));
		conn->InitConnectin(m_eventLoop);
		m_server->AddConnection(conn);
		IncreaseFdCount();
	}
	else if (strcmp(buf, "Close") == 0)
	{
		ev_break(m_eventLoop, EVBREAK_ALL);
	}
}

int EventLoopThread::GetListenFdCount()
{
	boost::unique_lock<boost::mutex> lock(m_mutex);
	return m_fdCount;
}

void EventLoopThread::IncreaseFdCount()
{
	boost::unique_lock<boost::mutex> lock(m_mutex);
	m_fdCount++;
}

void EventLoopThread::DecreaseFdCount()
{
	boost::unique_lock<boost::mutex> lock(m_mutex);
	m_fdCount--;
}
