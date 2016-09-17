/*
 * Connection.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: root
 */


#include "Server.h"
#include "EventLoopThread.h"
#include "Connection.h"

using namespace fm;

void SocketDataArrived(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	Connection* conn = (Connection*)watcher->data;
	conn->ReadAndDisptchMessage();
}


Connection::Connection(int fd, struct sockaddr_in addr,Server* server,EventLoopThread* eventLoopThread):
m_clientFd(fd),m_clientAddr(addr),m_eventLoop(NULL),m_server(server),m_eventLoopThread(eventLoopThread)
{
}

Connection::~Connection()
{
	if (m_eventLoop)
	{
		ev_io_stop(m_eventLoop, &m_ev_read);
	}
	close(m_clientFd);
	std::cout<<"Connection::~Connection"<<std::endl;
}

void Connection::InitConnectin(struct ev_loop* loop)
{
	m_eventLoop = loop;
	m_ev_read.data = this;

	ev_io_init(&m_ev_read, SocketDataArrived, m_clientFd, EV_READ);
	ev_io_start(m_eventLoop, &m_ev_read);
}

bool Connection::ReadAndDisptchMessage()
{
	try{
		Message::Header header;
		int read_size = read(m_clientFd,&header,sizeof(Message::Header));
		if (read_size < 0)
		{
			std::cout<<"Read Message Header Error"<<std::endl;
			return false;
		} 
		else if (read_size == 0)
		{
			std::cout<<"Client Close"<<std::endl;
			close(m_clientFd);
			m_eventLoopThread->DecreaseFdCount();
			m_server->RemoveConnection(shared_from_this());
			return false;
		}
		if (read_size < sizeof(Message::Header))
		{
			std::cout<<"Message header is too short"<<std::endl;
			return false;
		}
		if (!Message::Verify((Message*)&header)) 
		{
			std::cout<<"Invalid tcp message header"<<std::endl;
			return false;
		}
		std::cout<<"Receive Message Body Length Is: "<<header.length<<std::endl;
		std::cout<<"Receive Message ID Length Is: "<<header.msgid<<std::endl;

		Message* curMessage = Message::Allocate(header.msgid, header.length);
		int remainLength = header.length + sizeof(Message) - sizeof(Message::Header);
		read_size = read(m_clientFd,((unsigned char*)curMessage)+sizeof(Message::Header),remainLength);
		
		if (read_size < 0) {
			std::cout<<"Read Message Body Error"<<std::endl;
			close(m_clientFd);
			return false;

		}
		std::string messageStr((char*)&(curMessage->body[0]));
		std::cout<<"accept data from client: "<<messageStr<<std::endl;
		std::cout<<fm::Time::Now().FormatString()<<":accept data from client("<<inet_ntoa(m_clientAddr.sin_addr)<<":"<<
			m_clientAddr.sin_port<<") :"<<messageStr<<std::endl;

		if (!m_server->AddMessage(curMessage))
		{
			Message::Free(curMessage);
		}
	}catch(std::exception& ex){
		std::cout<<"error:"<<ex.what()<<std::endl;
	}
	return true;
}

