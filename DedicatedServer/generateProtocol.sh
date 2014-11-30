export GOPATH=$HOME/Golang
cd protocol
if [ ! -d "$GOPATH/src/protocol" ]; then
    mkdir $GOPATH/src/protocol
fi
export PATH=$PATH:$GOPATH/bin
protoc --go_out=$GOPATH/src/protocol req_register.proto request.proto
