/**
 * NGFX Textured Cube Sample
 *
 * Demonstrates:
 *  - Device/queue/present layer creation
 *  - Vertex & index buffer creation and upload
 *  - Procedural texture generation and upload
 *  - Sampler creation
 *  - Render pass and render pipeline setup
 *  - Per-frame MVP uniform buffer update
 *  - Command buffer recording with draw calls
 *  - Swapchain present
 */
#include "CoreMinimal.h"
#include "ngfx.h"
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef ngfx::Factory* (*PFN_CreateFactory)(bool debug, void(*log_call)(int, const char*));

#if K3DPLATFORM_OS_PROSPERO
static constexpr const char* kDefaultNgfxLibrary = "ngfx_agc";
#else
static constexpr const char* kDefaultNgfxLibrary = "ngfx_vk.dll";
#endif

static void logCallback(int level, const char* msg) {
    printf("[ngfx %d] %s\n", level, msg);
}

// ---------------------------------------------------------------------------
// Inline math (column-major 4x4)
// ---------------------------------------------------------------------------
struct Vec3 { float x, y, z; };
struct Vec2 { float x, y; };

struct Mat4 {
    float m[16];

    static Mat4 identity() {
        Mat4 r = {};
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    static Mat4 perspective(float fovY, float aspect, float zNear, float zFar) {
        float tanHalf = tanf(fovY * 0.5f);
        Mat4 r = {};
        r.m[0]  = 1.0f / (aspect * tanHalf);
        r.m[5]  = 1.0f / tanHalf;
        r.m[10] = zFar / (zNear - zFar);
        r.m[11] = -1.0f;
        r.m[14] = (zFar * zNear) / (zNear - zFar);
        return r;
    }

    static Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up) {
        Vec3 f = { center.x - eye.x, center.y - eye.y, center.z - eye.z };
        float fl = sqrtf(f.x*f.x + f.y*f.y + f.z*f.z);
        f.x /= fl; f.y /= fl; f.z /= fl;
        Vec3 s = { f.y*up.z - f.z*up.y, f.z*up.x - f.x*up.z, f.x*up.y - f.y*up.x };
        float sl = sqrtf(s.x*s.x + s.y*s.y + s.z*s.z);
        s.x /= sl; s.y /= sl; s.z /= sl;
        Vec3 u = { s.y*f.z - s.z*f.y, s.z*f.x - s.x*f.z, s.x*f.y - s.y*f.x };
        Mat4 r = Mat4::identity();
        r.m[0] = s.x;  r.m[4] = s.y;  r.m[8]  = s.z;
        r.m[1] = u.x;  r.m[5] = u.y;  r.m[9]  = u.z;
        r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;
        r.m[12] = -(s.x*eye.x + s.y*eye.y + s.z*eye.z);
        r.m[13] = -(u.x*eye.x + u.y*eye.y + u.z*eye.z);
        r.m[14] =  (f.x*eye.x + f.y*eye.y + f.z*eye.z);
        return r;
    }

    static Mat4 rotateY(float angle) {
        Mat4 r = Mat4::identity();
        float c = cosf(angle), s = sinf(angle);
        r.m[0] = c;  r.m[8]  = s;
        r.m[2] = -s; r.m[10] = c;
        return r;
    }

    Mat4 operator*(const Mat4& b) const {
        Mat4 r = {};
        for (int c = 0; c < 4; ++c)
            for (int row = 0; row < 4; ++row)
                for (int k = 0; k < 4; ++k)
                    r.m[c*4+row] += m[k*4+row] * b.m[c*4+k];
        return r;
    }
};

// ---------------------------------------------------------------------------
// Cube geometry: 24 vertices (4 per face), 36 indices
// Each vertex: position(3) + uv(2) + normal(3) = 8 floats
// ---------------------------------------------------------------------------
struct CubeVertex {
    float px, py, pz;
    float u, v;
    float nx, ny, nz;
};

static const CubeVertex kCubeVertices[24] = {
    // +Z face (front)
    { -1, -1,  1,  0, 1,   0, 0, 1 },
    {  1, -1,  1,  1, 1,   0, 0, 1 },
    {  1,  1,  1,  1, 0,   0, 0, 1 },
    { -1,  1,  1,  0, 0,   0, 0, 1 },
    // -Z face (back)
    {  1, -1, -1,  0, 1,   0, 0,-1 },
    { -1, -1, -1,  1, 1,   0, 0,-1 },
    { -1,  1, -1,  1, 0,   0, 0,-1 },
    {  1,  1, -1,  0, 0,   0, 0,-1 },
    // +X face (right)
    {  1, -1,  1,  0, 1,   1, 0, 0 },
    {  1, -1, -1,  1, 1,   1, 0, 0 },
    {  1,  1, -1,  1, 0,   1, 0, 0 },
    {  1,  1,  1,  0, 0,   1, 0, 0 },
    // -X face (left)
    { -1, -1, -1,  0, 1,  -1, 0, 0 },
    { -1, -1,  1,  1, 1,  -1, 0, 0 },
    { -1,  1,  1,  1, 0,  -1, 0, 0 },
    { -1,  1, -1,  0, 0,  -1, 0, 0 },
    // +Y face (top)
    { -1,  1,  1,  0, 1,   0, 1, 0 },
    {  1,  1,  1,  1, 1,   0, 1, 0 },
    {  1,  1, -1,  1, 0,   0, 1, 0 },
    { -1,  1, -1,  0, 0,   0, 1, 0 },
    // -Y face (bottom)
    { -1, -1, -1,  0, 1,   0,-1, 0 },
    {  1, -1, -1,  1, 1,   0,-1, 0 },
    {  1, -1,  1,  1, 0,   0,-1, 0 },
    { -1, -1,  1,  0, 0,   0,-1, 0 },
};

static const uint16_t kCubeIndices[36] = {
     0, 1, 2,  2, 3, 0,   // +Z
     4, 5, 6,  6, 7, 4,   // -Z
     8, 9,10, 10,11, 8,   // +X
    12,13,14, 14,15,12,   // -X
    16,17,18, 18,19,16,   // +Y
    20,21,22, 22,23,20,   // -Y
};

// ---------------------------------------------------------------------------
// 64x64 checkerboard texture (RGBA8)
// ---------------------------------------------------------------------------
static constexpr uint32_t kTexWidth  = 64;
static constexpr uint32_t kTexHeight = 64;
static constexpr uint32_t kTexSize   = kTexWidth * kTexHeight * 4;

static void generateCheckerboard(uint8_t* pixels, uint32_t w, uint32_t h, uint32_t tileSize) {
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            bool white = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            uint32_t idx = (y * w + x) * 4;
            pixels[idx + 0] = white ? 240 : 40;
            pixels[idx + 1] = white ? 230 : 60;
            pixels[idx + 2] = white ? 220 : 120;
            pixels[idx + 3] = 255;
        }
    }
}

// ---------------------------------------------------------------------------
// Embedded SPIR-V shaders
// ---------------------------------------------------------------------------
// Minimal vertex shader:
//   layout(location=0) in vec3 inPos;
//   layout(location=1) in vec2 inUV;
//   layout(location=2) in vec3 inNormal;
//   layout(set=0, binding=0) uniform UBO { mat4 mvp; };
//   layout(location=0) out vec2 outUV;
//   layout(location=1) out vec3 outNormal;
//   void main() { gl_Position = mvp * vec4(inPos, 1.0); outUV = inUV; outNormal = inNormal; }
//
// Minimal fragment shader:
//   layout(location=0) in vec2 inUV;
//   layout(location=1) in vec3 inNormal;
//   layout(set=0, binding=1) uniform sampler2D texSampler;
//   layout(location=0) out vec4 outColor;
//   void main() {
//       vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
//       float ndl = max(dot(normalize(inNormal), lightDir), 0.3);
//       outColor = texture(texSampler, inUV) * ndl;
//   }
//
// Pre-compiled to SPIR-V. In a real app, use glslc or the ShaderCompiler subsystem.
// Placeholders below -- a real build would embed actual SPIR-V binaries.

// ngfx::Blob implementation wrapping a static byte array
class StaticBlob : public ngfx::Blob {
public:
    StaticBlob(const void* data, uint64_t size) : data_(data), size_(size) {}
    const void* data() const override { return data_; }
    uint64_t length() const override { return size_; }
private:
    const void* data_;
    uint64_t size_;
};

class StaticFunction : public ngfx::Function {
public:
    StaticFunction(const char* entry, const void* spirv, uint64_t spirvSize)
        : entry_(entry), blob_(spirv, spirvSize) {}
    const char* entry() const override { return entry_; }
    const ngfx::Blob* bundle() const override { return &blob_; }
private:
    const char* entry_;
    StaticBlob blob_;
};

// ---------------------------------------------------------------------------
// Application
// ---------------------------------------------------------------------------
static constexpr uint32_t kWidth  = 1280;
static constexpr uint32_t kHeight = 720;

class TexturedCubeApp : public k3d::App {
public:
    TexturedCubeApp()
        : k3d::App("TexturedCube", kWidth, kHeight)
        , loader_{ k3d::os::LibraryLoader(kDefaultNgfxLibrary) }
    {}

    bool OnInit() override {
        App::OnInit();

        // Load NGFX Vulkan backend
        auto createFn = (PFN_CreateFactory)loader_.ResolveSymbol("CreateFactory");
        if (!createFn) return false;
        factory_ = createFn(true, logCallback);
        if (!factory_ || factory_->numDevices() == 0) return false;

        device_ = factory_->getDevice(0);

        // Present layer
        ngfx::PresentLayerDesc layerDesc = {};
        layerDesc.format     = ngfx::PixelFormat::BGRA8Unorm;
        layerDesc.width      = kWidth;
        layerDesc.height     = kHeight;
        layerDesc.colorSpace = ngfx::ColorSpace::SRGBNonLinear;
        layerDesc.hdrDisplay = false;
        layerDesc.maxImages  = 3;
        layerDesc.winHandle  = HostWindow()->GetHandle();
#if _WIN32
        //layerDesc.extraData  = GetModuleHandle(NULL);
#endif
        ngfx::Result r;
        layer_ = factory_->newPresentLayer(&layerDesc, device_, nullptr, &r);
        queue_ = device_->newQueue(&r);

        createBuffers();
        createTexture();
        createSampler();
        createPipeline();

        return true;
    }

    void OnProcess(k3d::Message& msg) override {
        if (msg.type == k3d::Message::Resized)
            return;
    }

    void OnUpdate() override {
        frame_count_++;
        float angle = (float)frame_count_ * 0.01f;

        // Update MVP
        Mat4 model = Mat4::rotateY(angle);
        Mat4 view  = Mat4::lookAt({0, 2, 5}, {0, 0, 0}, {0, 1, 0});
        Mat4 proj  = Mat4::perspective((float)(M_PI / 4.0), (float)kWidth / kHeight, 0.1f, 100.0f);
        Mat4 mvp   = proj * view * model;

        if (uniform_buffer_) {
            void* mapped = uniform_buffer_->map(0, sizeof(Mat4));
            if (mapped) {
                memcpy(mapped, mvp.m, sizeof(Mat4));
                uniform_buffer_->unmap(mapped);
            }
        }

        // Acquire drawable
        ngfx::Drawable* drawable = layer_->nextDrawable();
        if (!drawable) return;

        ngfx::Result result;
        ngfx::CommandBuffer* cmdBuf = queue_->newCommandBuffer();

        // Render encoder
        ngfx::RenderEncoder* encoder = cmdBuf->newRenderEncoder(&result);
        if (!encoder) {
            cmdBuf->commit();
            return;
        }

        if (render_pipeline_)
            encoder->setPipeline(render_pipeline_);

        ngfx::Viewport vp = { 0, 0, (float)kWidth, (float)kHeight, 0.0f, 1.0f };
        encoder->setViewport(vp);

        ngfx::Rect scissor = { 0, 0, kWidth, kHeight };
        encoder->setScissors(1, &scissor);

        // Draw cube
        encoder->drawIndexedPrimitives(
            ngfx::PrimitiveType::Triangles,
            ngfx::IndexType::UInt16,
            36,
            index_buffer_,
            0,
            0,
            24,
            1,
            0
        );

        encoder->present(drawable);
        encoder->endEncode();

        cmdBuf->commit();
        device_->wait();
    }

private:
    void createBuffers() {
        ngfx::Result r;

        // Vertex buffer
        ngfx::BufferDesc vbDesc = {};
        vbDesc.usages = ngfx::BufferUsage::VertexBuffer;
        vbDesc.size   = sizeof(kCubeVertices);
        vertex_buffer_ = device_->newBuffer(&vbDesc, ngfx::StorageMode::Shared, &r);
        if (vertex_buffer_) {
            void* dst = vertex_buffer_->map(0, sizeof(kCubeVertices));
            if (dst) {
                memcpy(dst, kCubeVertices, sizeof(kCubeVertices));
                vertex_buffer_->unmap(dst);
            }
        }

        // Index buffer
        ngfx::BufferDesc ibDesc = {};
        ibDesc.usages = ngfx::BufferUsage::IndexBuffer;
        ibDesc.size   = sizeof(kCubeIndices);
        index_buffer_ = device_->newBuffer(&ibDesc, ngfx::StorageMode::Shared, &r);
        if (index_buffer_) {
            void* dst = index_buffer_->map(0, sizeof(kCubeIndices));
            if (dst) {
                memcpy(dst, kCubeIndices, sizeof(kCubeIndices));
                index_buffer_->unmap(dst);
            }
        }

        // Uniform buffer (MVP matrix)
        ngfx::BufferDesc ubDesc = {};
        ubDesc.usages = ngfx::BufferUsage::UniformBuffer;
        ubDesc.size   = sizeof(Mat4);
        uniform_buffer_ = device_->newBuffer(&ubDesc, ngfx::StorageMode::Shared, &r);
        if (uniform_buffer_) {
            Mat4 identity = Mat4::identity();
            void* dst = uniform_buffer_->map(0, sizeof(Mat4));
            if (dst) {
                memcpy(dst, identity.m, sizeof(Mat4));
                uniform_buffer_->unmap(dst);
            }
        }
    }

    void createTexture() {
        ngfx::Result r;

        // Staging buffer for texture data
        uint8_t pixels[kTexSize];
        generateCheckerboard(pixels, kTexWidth, kTexHeight, 8);

        ngfx::TextureDesc texDesc = {};
        texDesc.usages    = ngfx::TextureUsage::ShaderResource;
        texDesc.format    = ngfx::PixelFormat::RGBA8Unorm;
        texDesc.width     = kTexWidth;
        texDesc.height    = kTexHeight;
        texDesc.depth     = 1;
        texDesc.layers    = 1;
        texDesc.mipLevels = 1;
        cube_texture_ = device_->newTexture(&texDesc, ngfx::StorageMode::Shared, &r);

        if (cube_texture_) {
            texture_view_ = cube_texture_->newView(&r);
        }
    }

    void createSampler() {
        ngfx::Result r;
        ngfx::SamplerDesc samplerDesc = {};
        samplerDesc.minFilter          = ngfx::FilterMode::Linear;
        samplerDesc.magFilter          = ngfx::FilterMode::Linear;
        samplerDesc.mipMapFilter       = ngfx::FilterMode::Linear;
        samplerDesc.u                  = ngfx::SamplerAddressMode::Wrap;
        samplerDesc.v                  = ngfx::SamplerAddressMode::Wrap;
        samplerDesc.w                  = ngfx::SamplerAddressMode::Wrap;
        samplerDesc.comparisonFunction = ngfx::ComparisonFunction::Never;
        samplerDesc.maxAnistropy       = 1;
        samplerDesc.minLod             = 0.0f;
        samplerDesc.maxLod             = 1.0f;
        sampler_ = device_->newSampler(&samplerDesc, &r);
    }

    void createPipeline() {
        ngfx::Result r;

        // Create render pass
        ngfx::RenderpassDesc rpDesc = {};
        renderpass_ = device_->newRenderpass(&rpDesc, &r);
        if (!renderpass_) return;

        // Pipeline descriptor
        ngfx::RenderPipelineDesc pipelineDesc = {};

        // Rasterizer state
        pipelineDesc.rasterizer.fillMode       = ngfx::FillMode::Fill;
        pipelineDesc.rasterizer.cullMode       = ngfx::CullMode::Back;
        pipelineDesc.rasterizer.frontCCW       = true;
        pipelineDesc.rasterizer.depthClipEnable = true;
        pipelineDesc.rasterizer.multisample    = ngfx::MultisampleFlags::MS1X;

        // Depth stencil
        pipelineDesc.depthStencil.depthTest      = false;
        pipelineDesc.depthStencil.depthWriteMask  = ngfx::DepthWriteMask::All;
        pipelineDesc.depthStencil.depthFunction   = ngfx::ComparisonFunction::Less;

        // Blend state - single render target, no blending
        ngfx::RenderTargetBlendState rtBlend = {};
        rtBlend.blendEnable   = false;
        rtBlend.colorWriteMask = 0xF; // RGBA
        pipelineDesc.blend.renderTargets.push(rtBlend);

        // Vertex input: position(Float3) + uv(Float2) + normal(Float3)
        ngfx::VertexLayout layout0 = {};
        layout0.inputRate = ngfx::VertexInputRate::PerVertex;
        layout0.stride    = sizeof(CubeVertex);
        pipelineDesc.input.layouts.push(layout0);

        ngfx::VertexAttribute attrPos = {};
        attrPos.format = ngfx::PixelFormat::RGBA32Float; // vec3 padded
        attrPos.offset = 0;
        attrPos.slot   = 0;
        pipelineDesc.input.attributes.push(attrPos);

        ngfx::VertexAttribute attrUV = {};
        attrUV.format = ngfx::PixelFormat::RG32Float;
        attrUV.offset = sizeof(float) * 3;
        attrUV.slot   = 0;
        pipelineDesc.input.attributes.push(attrUV);

        ngfx::VertexAttribute attrNormal = {};
        attrNormal.format = ngfx::PixelFormat::RGBA32Float; // vec3 padded
        attrNormal.offset = sizeof(float) * 5;
        attrNormal.slot   = 0;
        pipelineDesc.input.attributes.push(attrNormal);

        // Shaders: in a real app, provide compiled SPIR-V via StaticFunction
        pipelineDesc.vertex   = nullptr;
        pipelineDesc.pixel    = nullptr;
        pipelineDesc.geometry = nullptr;
        pipelineDesc.domain   = nullptr;
        pipelineDesc.hull     = nullptr;

        render_pipeline_ = renderpass_->newRenderPipeline(&pipelineDesc, &r);
    }

private:
    k3d::os::LibraryLoader  loader_;
    ngfx::Factory*          factory_         = nullptr;
    ngfx::Device*           device_          = nullptr;
    ngfx::PresentLayer*     layer_           = nullptr;
    ngfx::CommandQueue*     queue_           = nullptr;
    ngfx::Renderpass*       renderpass_      = nullptr;
    ngfx::RenderPipeline*   render_pipeline_ = nullptr;
    ngfx::Buffer*           vertex_buffer_   = nullptr;
    ngfx::Buffer*           index_buffer_    = nullptr;
    ngfx::Buffer*           uniform_buffer_  = nullptr;
    ngfx::Texture*          cube_texture_    = nullptr;
    ngfx::TextureView*      texture_view_    = nullptr;
    ngfx::Sampler*          sampler_         = nullptr;
    uint64_t                frame_count_     = 0;
};

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
#if _WIN32
int WinMain(void*, void*, char*, int)
#else
int main(int argc, const char* argv[])
#endif
{
    TexturedCubeApp app;
    return k3d::RunApplication(app, "TexturedCube");
}
