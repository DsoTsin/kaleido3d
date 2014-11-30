#include "Kaleido3D.h"
#include <Network/TCPClient.h>

#include "request.pb.h"
#include "req_register.pb.h"

#pragma comment(lib, "Ws2_32.lib")
using namespace k3d;

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	TCPClient client;
	Ref<INetConn> conn = client.ConnectTo(IPv4Address("112.74.110.22:24500"));

	protocol::meta_request  _request;
	protocol::req_register  *_register = new protocol::req_register;

	_register->set_username("QINZHOU");
	_register->set_password("12345467");
	_register->set_sex(0);
	_register->set_email("dsotsen@gmail.com");

	_request.set_type(::protocol::meta_request_Type_REGISTER);
	_request.set_allocated_registerinfo(_register);

	std::string message;
	_request.SerializeToString(&message);

	conn->Send(message);

	google::protobuf::ShutdownProtobufLibrary();
	::exit(0);
	return 0;
}
