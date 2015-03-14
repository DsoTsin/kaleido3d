//
//  StaticMesh.swift
//  kaleido3d
//
//  Created by QinZhou on 15/3/14.
//  Copyright (c) 2015年 TsinStudio. All rights reserved.
//

import Foundation

public class StaticMesh : IAsset {
    
    private var mVertexBuffer : UnsafeMutableBufferPointer<Byte>?
    private var mIndexBuffer : UnsafeMutableBufferPointer<Byte>?
    
    func load(fileName : String, fileType : String) -> Bool {
        var fileRoot = NSBundle.mainBundle().pathForResource( fileType, ofType: fileType)
        var contents: AnyObject? = NSData.dataWithContentsOfMappedFile(fileRoot!)
        return true
    }
}