#pragma once

struct IReflectable {
	virtual IReflectable * Reflect() = 0;
};