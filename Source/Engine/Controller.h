#pragma once
#include "Camera.h"
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
		virtual void SetCamera(std::shared_ptr<Camera> & camptr);

		//virtual void HandleMessage(SharedAppMsgHandlerPtr  msg);

	protected:

		std::shared_ptr<Camera> m_CameraPtr;
	};

	typedef std::shared_ptr<Controller> SharedControllerPtr;
}