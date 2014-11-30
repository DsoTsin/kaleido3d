#pragma once
#ifndef __Looper_h__
#define __Looper_h__

namespace k3d {

	class Looper {
	public:
		Looper();

		virtual ~Looper();

		void StartLooper();
	};
}

#endif