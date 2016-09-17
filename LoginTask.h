#ifndef __LOGINTASK_H_
#define __LOGINTASK_H_

#include "Commondef.h"
#include <boost/thread.hpp>
#include <list>

using namespace fm;

class LoginTask : public ThreadTask
{
public:
	LoginTask(std::string name, std::string keyword);

	~LoginTask();

	virtual void Execute();
private:
	std::string m_name;
	std::string m_keyword;
};

#endif
