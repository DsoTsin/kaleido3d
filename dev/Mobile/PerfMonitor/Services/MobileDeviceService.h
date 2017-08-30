#pragma once
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include "AndroidProfiler/Proto/cpu.grpc.pb.h"

class Client
{
public:
    Client()
    {
        grpc::CreateChannel("localhost:12389",
            grpc::InsecureChannelCredentials());
    }
};