//
//  PassiveSocket.swift
//  TestSwiftyCocoa
//
//  Created by Helge Hess on 6/11/14.
//  Copyright (c) 2014 Always Right Institute. All rights reserved.
//

import Darwin
import Dispatch

public typealias PassiveSocketIPv4 = PassiveSocket<sockaddr_in>

/*
 * Represents a STREAM server socket based on the standard Unix sockets library.
 *
 * A passive socket has exactly one address, the address the socket is bound to.
 * If you do not bind the socket, the address is determined after the listen()
 * call was executed through the getsockname() call.
 *
 * Note that if the socket is bound it's still an active socket from the
 * system's PoV, it becomes an passive one when the listen call is executed.
 *
 * Sample:
 *
 *   let socket = PassiveSocket(address: sockaddr_in(port: 4242))
 *
 *   socket.listen(dispatch_get_global_queue(0, 0), backlog: 5) {
 *     println("Wait, someone is attempting to talk to me!")
 *     $0.close()
 *     println("All good, go ahead!")
 *   }
 */
public class PassiveSocket<T: SocketAddress>: Socket<T> {
  
  public var backlog      : Int? = nil
  public var isListening  : Bool { return backlog != nil }
  public var listenSource : dispatch_source_t? = nil
  
  /* init */
  // The overloading behaviour gets more weird every release?

  override public init(fd: Int32?) {
    // required, otherwise the convenience one fails to compile
    super.init(fd: fd)
  }
  
  public convenience init(type: Int32 = SOCK_STREAM) {
    // NOTE: this is a DUPE to Socket. It fails to inherit from Socket<T>
    // and for b5 another dupe below
    let lfd = socket(T.domain, type, 0)
    var fd:  Int32?
    if lfd != -1 {
      fd = lfd
    }
    else {
      // This is lame. Would like to 'return nil' ...
      // TBD: How to do proper error handling in Swift?
      println("Could not create socket.")
    }
    
    self.init(fd: fd)
  }
  
  public convenience init(address: T) {
    // does not work anymore in b5?: I again need to copy&paste
    // self.init(type: SOCK_STREAM)
    // DUPE:
    let lfd = socket(T.domain, SOCK_STREAM, 0)
    var fd:  Int32?
    if lfd != -1 {
      fd = lfd
    }
    else {
      // This is lame. Would like to 'return nil' ...
      // TBD: How to do proper error handling in Swift?
      println("Could not create socket.")
    }

    self.init(fd: fd)
    
    if isValid {
      reuseAddress = true
      if !bind(address) {
        close() // TBD: how to signal error state in Swift?
      }
    }
  }
  
  /* proper close */
  
  override public func close() {
    if listenSource != nil {
      dispatch_source_cancel(listenSource)
      listenSource = nil
    }
    super.close()
  }
  
  /* start listening */
  
  public func listen(backlog: Int = 5) -> Bool {
    if !isValid {
      return false
    }
    if isListening {
      return true
    }
    
    let rc = Darwin.listen(fd!, Int32(backlog))
    if (rc != 0) {
      return false
    }
    self.backlog       = backlog
    self.isNonBlocking = true
    return true
  }
  
  typealias TypedActiveSocket = ActiveSocket<T>
  
  public func listen(queue: dispatch_queue_t, backlog: Int = 5,
                     accept: ( TypedActiveSocket ) -> Void)
    -> Bool
  {
    if !isValid {
      return false
    }
    if isListening {
      return false
    }
    
    /* setup GCD dispatch source */
    
    listenSource = dispatch_source_create(
      DISPATCH_SOURCE_TYPE_READ,
      UInt(fd!), // is this going to bite us?
      0,
      queue
    )
    
    if listenSource != nil {
      let lfd = fd! // please the closure and don't capture self
      
      listenSource!.onEvent { _, _ in
        do {
          // FIXME: tried to encapsulate this in a sockaddrbuf which does all
          //        the ptr handling, but it ain't work (autoreleasepool issue?)
          var baddr    = T()
          var baddrlen = socklen_t(baddr.len)
          
          let newFD = withUnsafeMutablePointer(&baddr) {
            ptr -> Int32 in
            let bptr = UnsafeMutablePointer<sockaddr>(ptr) // cast
            return Darwin.accept(lfd, bptr, &baddrlen);// buflenptr)
          }
          
          if newFD != -1 {
            // we pass over the queue, seems convenient. Not sure what kind of
            // queue setup a typical server would want to have
            let newSocket =
              TypedActiveSocket(fd: newFD, remoteAddress: baddr, queue: queue)
            newSocket.isSigPipeDisabled = true
            
            accept(newSocket)
          }
          else if errno == EWOULDBLOCK {
            break
          }
          else { // great logging as Paul says
            println("Failed to accept() socket: \(self) \(errno)")
          }
          
        } while (true);
      }
      
      dispatch_resume(listenSource)
      
      let listenOK = listen(backlog: backlog)
      
      if (listenOK) {
        return true
      }
      else {
        dispatch_source_cancel(listenSource)
        listenSource = nil
      }
    }
    
    return false
  }
  
  
  /* description */
  
  override func descriptionAttributes() -> String {
    var s = super.descriptionAttributes()
    if isListening {
      s += " listening"
    }
    return s
  }
}
