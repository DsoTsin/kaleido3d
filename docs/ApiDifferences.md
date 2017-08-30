
| GFX Object\API | Vulkan | Direct3D12 | Metal |
|:-:|:-:|:-:|:-:|
|CommandBuffer|Secondary|Bundle|ParallelCommandEncoder|
|PipelineState|Pipeline|PipelineState/PipelineLibrary|PipelineState|
|BindingLayout|PipelineLayout|RootSignature|-|
|BindingGroup|DescriptorSet|DescriptorHeap|-|
|RenderPass|RenderPass|-|RenderPassDesc|
|SwapChain|SwapChain|SwapChain|MTLLayer|
|RenderTarget|Framebuffer & ImageView|Resource & RTV|MTLLayer|
|Barrier|MemoryBarrier|D3D12_RESOURCE_TRANSITION_BARRIER||
|Buffer|Buffer|Resource|MTLBuffer|
|Texture|Image|Resource|MTLTexture|
|BufferView|BufferView|DSV/UAV/SRV/RTV|-|
|TextureView|ImageView|DSV/UAV/SRV/RTV|-|
|MemoryAllocator|||MTLHeap|