````
    Copyright (c) 2014, Tsin Studio. All rights reserved.
    Copyright (c) 2014, Qin Zhou. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Neither the name of TSIN STUDIO nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
    OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.    
````

Introduction
=========

[![Join the chat at https://gitter.im/TsinStudio/kaleido3d](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/TsinStudio/kaleido3d?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
> **Main Feature**
> 
>* `Next Generation Graphics Library` 
>* Modern C++ Code (**`C++ 11`**)
>* Modern Graphics Renderer (`Metal`, `Vulkan`, `DirectX 12` and **`AZDO OpenGL`**)
>* **`Maya`** Digital Content Creation Tools

----------

Build Required
=========

> **Compiler Required:**
> 
>* [CMake 2.8+][1]
>* Visual Studio **2013** Update 3 +
>* Clang 3.4 +
>* G++ 4.8 +
>* [ThirdParty][4]

----------

Build Documents:
=========
    cd Document and run makedoc.bat

----------


Directories:
=========

> **Source:**
> 
>* ***Core***   *A Cross Platform Implementation Of `IO`, `SIMD` Math, Image And Thread*  
>* ***Renderer***   Implement Include OpenGL (`With NV Extensions: glCommandList..`), D3D12 And `iOS` Metal
>*  **Engine**
>* **Launcher**
>* **Physics**
    
> **ThirdParty**
> 
>*  GLEW
>*  [PhysX 3.3.1][2]
>*  SDL2.0.3
>*  [rapidJson][3]
>*  [ProtoBuf][5]

> **Include**
>
>*  Interface
>*  SIMD Math Library
>* Template Library


> **Shader:**  *Effects Implemented With HLSL And GLSL*
    
> **DedicatedServer:** *A server application implemented by GoLang.*

> **Swift:** *A protype of low level renderer(`iOS Metal+Swift`).*

----------

Contact
=========
> If you have any suggestion, please contact me via [**sina weibo**](http://weibo.com/tsinstudio) or [**email**](mailto:dsotsen@gmail.com). 


[1]: http://www.cmake.org
[2]: https://developer.nvidia.com/gameworksdownload
[3]: https://github.com/miloyip/rapidjson
[4]: http://pan.baidu.com/s/1hqCdXFu
[5]: https://github.com/google/protobuf