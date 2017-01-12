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


| | Windows | Android | MacOS/iOS |
|:---:|:---:|:---:|:---:|
|CI Status|[![Build status](https://ci.appveyor.com/api/projects/status/bkqv6wbtyr4538hf?svg=true)](https://ci.appveyor.com/project/TsinStudio/kaleido3d)|[![Circle CI](https://circleci.com/gh/TsinStudio/kaleido3d.svg?style=svg)](https://circleci.com/gh/TsinStudio/kaleido3d) |[![Build Status](https://travis-ci.org/TsinStudio/kaleido3d.svg?branch=master)](https://travis-ci.org/TsinStudio/kaleido3d)|
|IDE| VS2015+ | Android Studio 2.2+| Xcode 8.1+ |
|OS Requirements| Win10 | Android 7.+| MacOS Sierra |
|Dependency| [ThirdParty][8]/CMake 3.4+ | [ThirdParty][8]/NDK r12+/Gradle | [ThirdParty][8]/CMake 3.4+ |

----------

Build Instructions
=========

>* **Windows**: make.bat
>* **Mac OS X**: ./make_macos.sh
>* **Android**: (./)gradlew build

---

Current Status
========

- RHI(Render Hardware Interface)

	* [x] Vulkan backend **ready**.
	* [ ] DirectX 12 backend **WIP**
	* [ ] Metal backend **WIP**

- Core.Platform

	* [x] Windows implementation ready.
	* [x] Android RendererView.
	* [ ] iOS/MacOS not initialized.

- Tools
	
	* [x] HLSL ShaderCompiler (D3DCompiler & GLSLANG)
	* [x] Maya exporter.

- Planned Samples
	
	* [x] Triangle(vk)
	* [x] Textured Cube 
	* [ ] Compute Shader
	* [ ] Shadow Mapping
	* [ ] Physically Based Shading
	* [ ] Deferred Shading
	* [ ] Multi-thread Rendering
	* [ ] Multi-GPU/CrossAdapter Rendering
	* [ ] Cross Shader Language Compiler

---

Current Architecture
=========

![arch](Document/architect_current.png)

---

Documents
=========

* [**Include**](Include/ReadMe.md): common definitions & template library
* [**Source.Core**](Source/Core/README.md) 
* [**Source.RHI**](Source/RHI/README.md): Implementation Include **Vulkan**, **Direct3D 12** And **Metal** API
* [**Source.Render**](Source/Renderer/README.md)
* [**Source.Tools.ShaderCompiler**](Source/Tools/ShaderGen/README.md) : cross shader language compiler and translator.
* **Source.Tools.MayaDcc** : maya plugin for engine assets exportation.
* **Source.UnitTest**: unit tests of engine modules
* [**ThirdParty**][8]
	*  [rapidJson][3]
	*  [glslang][7]
	*  [spir2cross][10]
	*  DXSDK


> Note: This project is under MIT License.
	
----------

RHI Sample Code
=======

## Draw a triangle

``` cpp
RHIDevices[Global Variable]
RHIQueues[Global]
-- PerThreadCode
rhi::PipelineDesc pipelineDesc = {shaders, raster, depthStencil...};
rhi::IPipelineState pState = device->NewPipelineState(pipelineDesc);
rhi::ICommandContext* gfxCmd = CommandContext::Begin(pDevice, pQueue);
	gfxCmd->Begin();
	gfxCmd->SetPipelineLayout(m_pl);
	rhi::Rect rect{ 0,0, (long)m_Viewport->GetWidth(), (long)m_Viewport->GetHeight() };
	gfxCmd->SetRenderTarget(pRT);
	gfxCmd->SetScissorRects(1, &rect);
	gfxCmd->SetViewport(rhi::ViewportDesc(m_Viewport->GetWidth(), m_Viewport->GetHeight()));
	gfxCmd->SetPipelineState(0, m_pPso);
	gfxCmd->SetIndexBuffer(m_TriMesh->IBO());
	gfxCmd->SetVertexBuffer(0, m_TriMesh->VBO());
	gfxCmd->DrawIndexedInstanced(rhi::DrawIndexedInstancedParam(3, 1));
	gfxCmd->EndRendering();
	gfxCmd->TransitionResourceBarrier(pRT->GetBackBuffer(), rhi::ERS_RenderTarget, rhi::ERS_Present);
	gfxCmd->End();
gfxCmd->FlushAndWait();
--
swapChain.Present(pQueue, pImage, pWindow[, semaphore])
```

---

Samples
=======

## 1.Triangle(Basic)

![Triangle Screenshot](Document/images/sample_triangle_screenshot.png)

## 2.TexturedCube(Basic)

![Cube Screenshot](Document/images/sample_texturedcube_screenshot.png)

---

Contact
=========
> If you have any suggestion, please contact me via [**email**](mailto:dsotsen@gmail.com). 

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
[10]: https://github.com/KhronosGroup/SPIRV-Cross
