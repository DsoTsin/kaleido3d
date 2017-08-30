#pragma once 
#include "WorkItem.h"
#include "WorkGroup.h"
#include "WorkQueue.h"
#include "Kaleido3D.h"

class K3D_CORE_API Dispatcher {
public:
	using Queue = ::Dispatch::WorkQueue;
	using Item = ::Dispatch::WorkItem;

	static void Dispatch(Queue &, Item &);

};