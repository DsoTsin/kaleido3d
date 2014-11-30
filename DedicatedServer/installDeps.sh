#!/bin/sh
export GOPATH=$HOME/Golang
go get github.com/TsinStudio/goprotobuf/{proto,protoc-gen-go}
go get github.com/go-sql-driver/mysql

go install github.com/TsinStudio/goprotobuf/{proto,protoc-gen-go}
go install github.com/go-sql-driver/mysql
