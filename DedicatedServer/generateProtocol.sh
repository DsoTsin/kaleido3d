export GOPATH=$HOME/Golang
cd protocol
if [ ! -d "$GOPATH/src/protocol" ]; then
    mkdir $GOPATH/src/protocol
fi
export PATH=$PATH:$GOPATH/bin
protoc --go_out=$GOPATH/src/protocol req_register.proto req_statistic.proto request.proto
protoc --cpp_out=./cpp req_register.proto req_statistic.proto request.proto
