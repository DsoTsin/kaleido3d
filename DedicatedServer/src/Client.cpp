//
//  main.cpp
//  ProtoBufEx
//
//  Created by dsotsen on 15/2/26.
//  Copyright (c) 2015年 dsotsen. All rights reserved.
//

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "req_register.pb.h"
#include "request.pb.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
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
    
    int sockfd;
    struct sockaddr_in server_addr;
    ::memset(&server_addr, 0, sizeof(server_addr));
    server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(24500)
    };
    
    ::inet_pton(AF_INET, "112.74.110.22", &server_addr.sin_addr);
    
    sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    int t = ::connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(t<0)
    {
        printf("error \n");
    }
    ::send(sockfd, message.c_str(), message.size(), 0);
    ::close(sockfd);
    google::protobuf::ShutdownProtobufLibrary();
    ::exit(0);
    return 0;
}
