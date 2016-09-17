#ifndef TASKFACTORY_H_
#define TASKFACTORY_H_

#include <CommonSDK/ThreadTask.h>
#include "Message.h"

using namespace fm;

class TaskFactory
{
public:
	~TaskFactory();

	static TaskFactory* Instance();

	ThreadTaskPtr CreateTask(Message* message);

protected:

	TaskFactory();
};

#endif
