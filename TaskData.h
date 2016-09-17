#ifndef __TASK_DATA_H_
#define __TASK_DATA_H_

enum TaskType
{
	LoginType
};

struct LoginTaskData
{
	char m_name[32];
	char m_keyword[32];
};

#endif
