#ifndef __SERVER_H_
#define __SERVER_H_

#include <ev.h>
#include <pthread.h>
#include <CommonSDK/ThreadPool.h>
#include "Commondef.h"

using namespace fm;

class EventLoopThread;

class Server
{
public:
	Server(const std::string& addr, const int port);
	~Server();

	void Initialize();

	void Run();

	void AcceptConnection();

	bool AddConnection(ConnectionPtr conn);

	void RemoveConnection(ConnectionPtr conn);

	bool AddMessage(Message* message);

	static bool MakeNonBlocking (int sfd);
private:
	void TerminateEventLoopThread();

	void SendMessage();

	static void DisptchMessageThread(Server* server);

	int SocketBind(const char* ip,int port);

	/**
	 * @brief 获取适当的eventloop. (保证每个eventloop线程里面监听的事件保持平衡)
     */
	EventLoopThread* GetCorrectEventLoop();
private:
	bool m_isExit;

	int m_listenfd;
	struct ev_loop *m_mainLoop;
	std::map<EventLoopThread*, int> m_loopReadfdMap;

	boost::mutex m_messageMutex;
	boost::condition_variable m_messageCondition;
	MessageList m_messageList;

	boost::mutex m_connectMutex;
	ConnectList m_connList;

	ThreadPtr m_thread;
	ThreadPoolPtr m_threadPool;

	std::string m_serverAddr;
	int m_serverPort;

	struct ev_io m_socket_watcher;
	struct ev_io m_stdin_watcher;
};

#endif

