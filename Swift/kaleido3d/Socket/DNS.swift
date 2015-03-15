//
//  DNS.swift
//  ARISockets
//
//  Created by Helge Hess on 7/3/14.
//  Copyright (c) 2014 Always Right Institute. All rights reserved.
//
import Darwin

func gethoztbyname<T: SocketAddress>
  (name : String, flags : Int32 = AI_CANONNAME,
   cb   : ( String, String?, T? ) -> Void)
{
  // Note: I can't just provide a name and a cb, swiftc will complain.
  var hints = addrinfo()
  hints.ai_flags  = flags  // AI_CANONNAME, AI_NUMERICHOST, etc
  hints.ai_family = T.domain
  
  var ptr     = UnsafeMutablePointer<addrinfo>(nil)
  let nullptr = UnsafeMutablePointer<addrinfo>.null()
  
  /* run lookup (synchronously, can be slow!) */
  // b3: (cs : CString) doesn't pick up the right overload?
  var rc = name.withCString { (cs : UnsafePointer<CChar>) -> Int32 in
    let ncs = UnsafePointer<CChar>.null()
    return getaddrinfo(cs, ncs, &hints, &ptr)
  }
  if rc != 0 {
    cb(name, nil, nil)
    return
  }
  
  /* copy results - we just take the first match */
  var cn   : String? = nil
  var addr : T?      = ptr.memory.address()
  if rc == 0 && ptr != nullptr {
    cn   = ptr.memory.canonicalName
    addr = ptr.memory.address()
  }
  
  /* free OS resources */
  freeaddrinfo(ptr)
  
  /* report results */
  cb(name, cn, addr)
}


/**
 * This is awkward :-) But it actually works. Though it is not what you want,
 * the address here should be dynamic depending on the domain of the C struct.
 *
 * Whatever, this runs:
 *   let hhost : String = "mail.google.com" // why is this necessary?
 *   gethostzbyname(hhost, flags: Int32(AI_CANONNAME)) {
 *     ( cn: String, infos: [ ( cn: String?, address: sockaddr_in? )]? ) -> Void
 *     in
 *     println("result \(cn): \(infos)")
 *   }
 *
 * TBD: The 'flags' has to be provided, otherwise the trailing closure is not
 *      detected right?
 */
func gethostzbyname<T: SocketAddress>
  (name : String, flags : Int32 = AI_CANONNAME,
   cb   : ( String, [ ( cn: String?, address: T? ) ]? ) -> Void
  ) -> Void
{
  // Note: I can't just provide a name and a cb, swiftc will complain.
  var hints = addrinfo()
  hints.ai_flags  = flags  // AI_CANONNAME, AI_NUMERICHOST, etc
  hints.ai_family = T.domain
  
  var ptr     = UnsafeMutablePointer<addrinfo>(nil)
  let nullptr = UnsafeMutablePointer<addrinfo>.null()
  
  /* run lookup (synchronously, can be slow!) */
  var rc = name.withCString { (cs : UnsafePointer<CChar>) -> Int32 in
    let ncs = UnsafePointer<CChar>.null()
    return getaddrinfo(cs, ncs, &hints, &ptr)
  }
  if rc != 0 {
    cb(name, nil)
    return
  }
  
  /* copy results - we just take the first match */
  typealias hapair = (cn: String?, address: T?)
  var results : Array<hapair>! = nil
  
  if rc == 0 && ptr != nullptr {
    var pairs = Array<hapair>()
    for info in ptr.memory {
      let pair : hapair = ( info.canonicalName, info.address() )
      pairs.append(pair)
    }
    results = pairs
  }
  
  /* free OS resources */
  freeaddrinfo(ptr)
  
  /* report results */
  
  cb(name, results)
}
