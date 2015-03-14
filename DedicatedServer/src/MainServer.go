package main

import (
	"fmt"
	"github.com/TsinStudio/goprotobuf/proto"
	"net"
	"os"
	"protocol"
	"encoding/json"
	"net/http"
	"io/ioutil"
	"database/sql"
    _ "github.com/go-sql-driver/mysql"
)

func main() {
	fmt.Println("Started Server")
	response,_ := http.Get("http://112.74.110.22/config/server.json")
	defer response.Body.Close()
	body,_ := ioutil.ReadAll(response.Body)
	
	var host map[string]string
	if err := json.Unmarshal(body, &host); err != nil {
		fmt.Println("host info error:", err)
		panic(err)
	} 

	hostInfo := host["host"]
	fmt.Println(hostInfo)
	
	listener, err := net.Listen("tcp", hostInfo)
	checkError(err)
	
    db, err := sql.Open("mysql", "root:111111bbb@/GameXDB")
    if err != nil {
        panic(err.Error())
    }
    defer db.Close()
	
	
	
	c := make(chan *protocol.MetaRequest)
	go func(){
		stmt, sqlErr := db.Prepare("INSERT INTO Users(username,passwd,email) VALUES(?, ?, ?)")
		if sqlErr != nil {
	        panic(err.Error())
		}
		defer stmt.Close()
		for{
			message := <-c
			writeValuesTofile(message, stmt)
		}
	}()
	
	for{
		if conn, err := listener.Accept(); err == nil {
			go handleProtoClient(conn, c)
		} else{
			continue
		}
	}
}

func handleProtoClient(conn net.Conn, c chan *protocol.MetaRequest){
	fmt.Println("Connection established")
	defer conn.Close()
	data := make([]byte, 4096)
	
	n,err:= conn.Read(data)
	checkError(err)
	fmt.Println("Decoding Protobuf message")
	protodata := new(protocol.MetaRequest) 
	if err = proto.Unmarshal(data[0:n], protodata); err!=nil {
		fmt.Println("Error protocol data!")
		return	
	}
	checkError(err)
	reqType := protodata.GetType()
	if reqType == protocol.MetaRequest_REGISTER {
		fmt.Println("Register")
		c <- protodata
	}
}

func writeValuesTofile(protocolData *protocol.MetaRequest, stmt *sql.Stmt){
	fmt.Println("adding one record...")
	if reqType := protocolData.GetType(); reqType==protocol.MetaRequest_REGISTER {
		userInfo := protocolData.GetRegisterInfo()

		UserName 	:= userInfo.GetUsername()
		PassWord 	:= userInfo.GetPassword()
		Email 		:= userInfo.GetEmail()
		
		fmt.Println(UserName, PassWord)

		if result, err := stmt.Exec(UserName, PassWord, Email); err==nil {
			fmt.Println("one record added to db", result) 
		} else {
			fmt.Println("one record failed to add to db..") 
		}
	}
}

func checkError(err error){
	if err != nil {
		fmt.Fprintf(os.Stderr, "Fatal error: %s", err.Error())
		os.Exit(1)
	}
}

