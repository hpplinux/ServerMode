#include "TaskFactory.h"
#include "TaskData.h"
#include "LoginTask.h"

TaskFactory::TaskFactory()
{

}

TaskFactory::~TaskFactory()
{

}

TaskFactory* TaskFactory::Instance()
{
	static TaskFactory taskFactory;
	return &taskFactory;
}

ThreadTaskPtr TaskFactory::CreateTask(Message* message)
{
	ThreadTaskPtr task = ThreadTaskPtr();
	if (!message)
	{
		return task;
	}
	switch(TaskType(message->header.msgid))
	{
	case LoginType:
		{
			const LoginTaskData& data = *((const LoginTaskData*)&message->body);
			task = ThreadTaskPtr(new LoginTask(data.m_name, data.m_keyword));
		}
		// to do
	}
	return task;
}


