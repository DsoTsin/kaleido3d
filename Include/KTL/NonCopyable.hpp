#pragma once 

class NonCopyable
{
protected:
	NonCopyable();

	NonCopyable(const NonCopyable &);
	NonCopyable(const NonCopyable &&);
	NonCopyable& operator=(const NonCopyable &);
};