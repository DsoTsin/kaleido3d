//
//  Asset.swift
//  kaleido3d
//
//  Created by QinZhou on 15/3/14.
//  Copyright (c) 2015年 TsinStudio. All rights reserved.
//

protocol IAsset {
    func load(fileName : String, fileType : String) -> Bool
}
