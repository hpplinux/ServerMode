/*
 * Serve.cpp
 *
 *  Created on: Oct 20, 2015
 *      Author: root
 */
#include "Server.h"
#include "Connection.h"
#include "TaskFactory.h"
#include "EventLoopThread.h"

using namespace fm;

#define MAXLINE     1024
#define LISTENQ     5
#define OPEN_MAX    1000
#define INFTIM      -1

void NewConnectionArrived(EV_P_ ev_io *w, int revents)
{
	//std::cout<<"Add_Connection"<<std::endl;
	Server* server =(Server*)(w->data);
	server->AcceptConnection();
}

void stdin_cb (EV_P_ ev_io *w, int revents)
{
	//Server* server =(Server*)(w->data);
	//server->SendMessage();
}

void SigHandler(struct ev_loop* loop, struct ev_signal* w, int)
{
	LOG_INFO ("server quit");
	ev_break(loop, EVBREAK_ALL);
}

Server::Server(const std::string& addr, const int port)
	:m_isExit(false),m_serverAddr(addr),m_serverPort(port)
{
	m_mainLoop = ev_loop_new(EVBACKEND_EPOLL);
	m_threadPool = CreateThreadPool(10);
}

Server::~Server()
{
	m_isExit = true;

	if (m_listenfd > 0)
		close(m_listenfd);
	TerminateEventLoopThread();
	m_connList.clear();

	AddMessage(NULL);
	if (m_thread)
	{
		m_thread->join();
	}
	
	for (MessageList::iterator it = m_messageList.begin(); it != m_messageList.end(); ++it)
	{
		Message::Free(*it);
	}
	m_threadPool->Terminate();
	m_threadPool->Join();

	std::cout<<"Server::~Server"<<std::endl;
}

bool Server::MakeNonBlocking(int sfd)
{
	int flags, s;
	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1)
	{
		return false;
	}
	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1)
	{
		return false;
	}
	return true;
}

EventLoopThread* Server::GetCorrectEventLoop()
{
	if (m_loopReadfdMap.size() == 0)
	{
		return NULL;
	}

	EventLoopThread* loopThread = m_loopReadfdMap.begin()->first;
	for (std::map<EventLoopThread*, int>::iterator it = m_loopReadfdMap.begin(); it != m_loopReadfdMap.end(); it++)
	{
		if(loopThread->GetListenFdCount() > it->first->GetListenFdCount())
		{
			loopThread = it->first;
		}
	}
	return loopThread;
}

void Server::AcceptConnection()
{
	EventLoopThread* loopThread = GetCorrectEventLoop();
	if (!loopThread)
	{
		return;
	}

	char buf[32] = "New Connection\0";
	if (write(m_loopReadfdMap[loopThread],buf,32) < 0)
	{
		LOG_INFO("Write Notify Error");
		return;
	}
}

void Server::Run()
{
	ev_signal ev_sigint;
	ev_signal_init(&ev_sigint, SigHandler, SIGINT);
	ev_signal_start(m_mainLoop, &ev_sigint);
	ev_run(m_mainLoop,0);
}

void Server::Initialize()
{
	m_listenfd = SocketBind(m_serverAddr.c_str(),m_serverPort);
	if (!MakeNonBlocking(m_listenfd))
	{
		LOG_INFO("Set Listen Socket NonBlock Failed");
		exit(0);
	}
	listen(m_listenfd,LISTENQ);  

	for (int i = 0; i < EVENT_LOOP_THREAD_MAX; i++)
	{
		int pip[2];
		if (pipe(pip) < 0)
		{
			LOG_INFO("Create pipe failed");
			exit(1);
		}
		EventLoopThread* eventLoop = new EventLoopThread(m_listenfd, pip[0], this);
		eventLoop->Initialize();
		m_loopReadfdMap[eventLoop] = pip[1];
	}


	m_socket_watcher.data = this;
	ev_io_init (&m_socket_watcher, NewConnectionArrived, m_listenfd, EV_READ);
	ev_io_start (m_mainLoop, &m_socket_watcher);


	m_thread = ThreadPtr(new boost::thread(boost::bind(Server::DisptchMessageThread, this)));
}

void Server::DisptchMessageThread(Server* server)
{
	bool isBreak = false;
	while(!isBreak)
	{
		{
			boost::unique_lock<boost::mutex> lock(server->m_messageMutex);
			if (server->m_messageList.size() == 0)
			{
				server->m_messageCondition.wait(lock);
				std::cout<<"condition is notify"<<std::endl;
			}
		}
		while(server->m_messageList.size() > 0)
		{
			Message* currentMessage = NULL;
			{
				boost::unique_lock<boost::mutex> lock(server->m_messageMutex);
				currentMessage = server->m_messageList.front();
				server->m_messageList.pop_front();
			}
			if (!currentMessage)
			{
				isBreak = true;
				break;
			}
			//
			ThreadTaskPtr task = TaskFactory::Instance()->CreateTask(currentMessage);
			if (task)
			{
				server->m_threadPool->PushTask(task);
			}
			Message::Free(currentMessage);
		}

	}
}
void Server::SendMessage()
{
	//boost::unique_lock<boost::mutex> lock(m_connectMutex);
	//char buf[2048] = {0};
	//int  rece_size;
	//rece_size = read(STDIN_FILENO,buf,2047);
	//buf[rece_size] = '\0';
	//for (ConnectList::iterator it = conn_list.begin(); it != conn_list.end(); ++it) {
	//	if (write((*it)->GetClientFd(),buf,rece_size) == -1){
	//		LOG_INFO("write error");
	//		ev_break(server_loop, EVBREAK_ALL);
	//	}
	//}
	//std::cout<<"send data to client:"<<buf<<std::endl;
}

bool Server::AddMessage(Message* message)
{
	boost::unique_lock<boost::mutex> lock(m_messageMutex);
	if (m_isExit && message)
	{
		return false;
	}
	m_messageList.push_back(message);
	m_messageCondition.notify_one();
	return true;
}

bool Server::AddConnection(ConnectionPtr conn)
{
	boost::unique_lock<boost::mutex> lock(m_connectMutex);
	if (m_isExit)
	{
		return false;
	}
	m_connList.push_back(conn);
	return true;
}

void Server::RemoveConnection(ConnectionPtr conn)
{
	boost::unique_lock<boost::mutex> lock(m_connectMutex);
	ConnectList::iterator it = std::find(m_connList.begin(),m_connList.end(),conn);
	if (it != m_connList.end())
	{
		std::cout<<"Delete Connection "<<std::endl;
		m_connList.erase(it);
	}
}

void Server::TerminateEventLoopThread()
{
	char buf[32] = "Close\0";
	std::map<EventLoopThread*, int>::iterator it;
	for(it = m_loopReadfdMap.begin(); it != m_loopReadfdMap.end(); it++)
	{
		if (write(it->second,buf,32) < 0)
		{
			LOG_INFO("Write Notify Error");
			continue;
		}
		delete it->first;
	}
	m_loopReadfdMap.clear();
}

int Server::SocketBind(const char* ip,int port)
{
    int  listenfd;
    struct sockaddr_in servaddr;
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd == -1)
    {
    	LOG_INFO("socket error:");
        exit(1);
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    if (bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1)
    {
    	LOG_INFO("bind error: ");
        exit(1);
    }
    return listenfd;
}


