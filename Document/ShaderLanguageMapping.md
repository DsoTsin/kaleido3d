# Shader Object mapping between GLSL and HLSL


| Binding Objects | image | buffer | imagebuffer | samplebuffer
|:-:|:-:|:-:|:-:|:-:|
|Buffer|||*||
|RWBuffer|||*||
|StructedBuffer||*||
|RWStructedBuffer||*||
|Texture|*|||
|RWTexture|*|||


# Uniform Buffers

* MaxSize: 64KB
* Memory Access: Coherent
* Memory Storage: Local Memory

# Texture Buffers

* MaxSize: 128MB or More
* Memory Access: Random
* Memory Storage: Global Texture Memory
* Fetch in Vertex Shader

> If you need unordered write access, use storage images for image resources and storage buffers for regular buffer resources. 

> Texel buffers are useful if you want to use some of the hardware texture formats for a 1d array. No more unpacking bytes from an int in the shader!

# Resource Binding in Vulkan

|Object||
|:-:|:-|
|**Sampled Image**|A descriptor type that represents an image view, and supports filtered (sampled) and unfiltered read-only acccess in a shader.|
|Sampler|An object that contains state that controls how sampled image data is sampled (or filtered) when accessed in a shader. Also a descriptor type describing the object. Represented by a VkSampler object. |
|Combined Image Sampler|A descriptor type that includes both a sampled image and a sampler.|
|**Storage Image**|A descriptor type that represents an image view, and supports unfiltered loads, stores, and atomics in a shader.|
|Uniform Buffer|A descriptor type that represents a buffer, and supports read-only access in a shader.|
|Dynamic Uniform Buffer|A uniform buffer whose offset is specified each time the uniform buffer is bound to a command buffer via a descriptor set.|
|Uniform Texel Buffer|A descriptor type that represents a buffer view, and supports unfiltered, formatted, read-only access in a shader.|
|**Storage Buffer**|A descriptor type that represents a buffer, and supports reads, writes, and atomics in a shader.|
|Storage Texel Buffer|A descriptor type that represents a buffer view, and supports unfiltered, formatted reads, writes, and atomics in a shader.|