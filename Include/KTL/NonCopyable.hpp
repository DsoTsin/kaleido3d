#pragma once 

K3D_COMMON_NS
{
class NonCopyable
{
protected:
	NonCopyable();

	NonCopyable(const NonCopyable &);
	NonCopyable(const NonCopyable &&);
	NonCopyable& operator=(const NonCopyable &);
};
}