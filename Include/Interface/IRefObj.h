#pragma once

class IRefObj
{
public:
	virtual uint32 AddRef() const = 0;
	virtual uint32 Release() const = 0;
	virtual uint32 GetRefCount() const = 0;
};