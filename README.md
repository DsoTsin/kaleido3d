Introduction 
=========


> **Main Feature**
> 
>* `Next Generation Graphics Library` 
>* Modern C++ Code (**`C++ 11`**)
>* Modern Graphics Renderer (`Metal`, `Vulkan`, `Direct3D 12`)
>* **`Maya`** Digital Content Creation Tools
>* **Task-Oriented**, support `multi-thread` rendering 
>* Support Windows, Android, iOS, MacOS & Linux.

----------

Prerequisites
=========

> **Windows:** [![Build status](https://ci.appveyor.com/api/projects/status/bkqv6wbtyr4538hf?svg=true)](https://ci.appveyor.com/project/TsinStudio/kaleido3d)
> 
>* [CMake 3.4 or later][1]
>* Visual Studio **2015** or later
>* [ThirdParty][8] `Git mirror on Github`
>* Win10


> **Android:** [![Build Status](https://travis-ci.org/TsinStudio/kaleido3d.svg?branch=master)](https://travis-ci.org/TsinStudio/kaleido3d)
> 
>* Android Studio 2.1 or later.
>* Android SDK N-preview or later.
>* NDK r12 beta or later.
>* [ThirdParty][8] `Git mirror on Github`



> **MacOS/iOS:**
> 
>* XCode 7 or later.
>* OS X EI Captain or later.


----------

Build Instructions:
=========

>* **Windows**: make.bat
>* **Mac OS X**: make.command
>* **Android**: cd Project/android && gradle

RHI Threading Model:
=========

``` cpp

RHIDevices[Global Variable]

RHIQueues[Global]

-- PerThreadCode

rhi::PipelineDesc pipelineDesc = {shaders, raster, depthStencil...};
rhi::IPipelineState pState = device->NewPipelineState(pipelineDesc);

rhi::ICommandContext cmd = CommandContext::Begin(pDevice, pQueue);
cmd.BindPipeline(pState);
cmd.BindDescriptorTable(pTable);
cmd.SetRenderTarget/SetRenderPass();
for(obj : scene)
{
	cmd.SetIndexBuffer(obj->ibo);
	cmd.SetVertexBuffer(obj->vbo);
	cmd.DrawIndex()/ExecuteCmdBuf();
}
cmd.End();
cmd.FlushAndWait();

--

swapChain.Present(pQueue, pImage, pWindow[, semaphore])


```

Directories:
=========

* **Source:** 
	* ***Core***   
		* *A Cross Platform Implementation Of `IO`, `SIMD` Math, Image And Thread*  
	* ***RHI***   
		* Implementation Include **D3D12**, **Vulkan** And `Metal` API
	* ***Render***
		* providing object-level rendering interfaces.
	* **Engine**
	* **Launcher**
	* **Physics**    
	* [**ThirdParty**][8]
		*  [PhysX 3.3.1][2]
		*  [rapidJson][3]
		*  [ProtoBuf][5]
		*  [Intel Thread Building Blocks][6]
		*  [glslang][7]
* **Include**
	*  Interface
	*  SIMD Math Library
	* Template Library


> Note: This project is under MIT License.
	
----------


Contact
=========
> If you have any suggestion, please contact me via [**sina weibo**](http://weibo.com/tsinstudio) or [**email**](mailto:dsotsen@gmail.com). 

Discuss [![Join the chat at https://gitter.im/TsinStudio/kaleido3d](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/TsinStudio/kaleido3d?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
=========



Please join the [gitter chat](https://gitter.im/TsinStudio/kaleido3d) to discuss on this project.
Framework development discussions and thorough bug reports are collected on [Issues](https://github.com/TsinStudio/kaleido3d/issues).



[1]: http://www.cmake.org
[2]: https://developer.nvidia.com/gameworksdownload
[3]: https://github.com/miloyip/rapidjson
[4]: https://code.csdn.net/tomicyo/kaleido3d_dep
[5]: https://github.com/google/protobuf
[6]: https://www.threadingbuildingblocks.org/
[7]: https://github.com/KhronosGroup/glslang
[8]: https://github.com/Tomicyo/kaleido3d_dep
[9]: https://vulkan.lunarg.com/
