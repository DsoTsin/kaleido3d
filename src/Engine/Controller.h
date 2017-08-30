#pragma once
#include "Camera.h"
#include <KTL/String.hpp>
#include <KTL/DynArray.hpp>
#include <KTL/SharedPtr.hpp>
//#include <dinput.h>
//#pragma comment(lib, "dinput8.lib")

namespace k3d {

	class Camera;

	///
	/// \brief The Controller class handles user inputs and
	/// updates the actors controlled, so it aggregates kApplicationMsgHandler
	///
	class Controller
	{
	public:

		Controller();
		virtual ~Controller();

		///
		/// \brief Set the camera controlled by controller
		/// \param[in, refCount] camptr
		///
		virtual void SetCamera(SharedPtr<Camera> & camptr);

		//virtual void HandleMessage(SharedAppMsgHandlerPtr  msg);

	protected:

        SharedPtr<Camera> m_CameraPtr;
	};

	typedef SharedPtr<Controller> SharedControllerPtr;
}