//
//  GameLoginController.swift
//  kaleido3d
//
//  Created by QinZhou on 15/3/14.
//  Copyright (c) 2015年 TsinStudio. All rights reserved.
//

import UIKit

class GameLoginController: UIViewController {

    @IBOutlet weak var mTextUserName: UITextField!
    @IBOutlet weak var mTextPassWord: UITextField!
    @IBOutlet weak var mButtonLogin: UIButton!
    @IBOutlet weak var mButtonRegister: UIButton!
    
    var socket : ActiveSocketIPv4?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do view setup here.
        socket = ActiveSocket<sockaddr_in>()
        if socket==nil {
            return
        }
        let host = "112.74.110.22"
        let port = Int(24500)
        let ok = socket?.connect(sockaddr_in(address: host, port: port)) {
            
        }
        
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        
    }
    
    @IBAction func onLogin(sender: UIButton) {
        NSLog("onLogin %@ %@", mTextUserName.text, mTextPassWord.text)
    }
    
    @IBAction func onRegister(sender: UIButton) {
        
    }
    
}
