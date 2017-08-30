#include "sl_parser.h"
#include "shaderlab_private.h"
#include <ngfx_shader_compiler.h>
#include <fstream>
#include <algorithm>
#include "CoreMinimal.h"
namespace shaderlab
{
    using namespace ngfx;

    ngfx::IShaderCompiler* LoadShCompiler(const char* dllName) {
        new k3d::os::LibraryLoader(dllName);
        return nullptr;
    }

    ngfx::Vec<String> Material::s_IncludeDirectories;

    class ShaderIncluder : public ngfx::IIncluder
    {
    public:
        ShaderIncluder() {}
        ~ShaderIncluder() {}

        ngfx::Result Open(IncludeType Type,
            const char* SrcFile, const char* RequestFile,
            void** ppData, uint64_t* pBytes,
            void* pUserData) override
        {
            ReadLocalFile(SrcFile, RequestFile, ppData, pBytes);
            return *ppData? Result::Ok : Result::Failed;
        }

        void Close(void *pData) override
        {
            delete[] pData;
        }

    private:
        void ReadLocalFile(const char* SrcFile, const char* RequestFile, void** ppData, uint64_t* pBytes)
        {
            // for inc in s_incs
            //      if exist inc + reqFile

            // Find a directory that works, using a reverse search of the include stack.
            for (auto it = Material::s_IncludeDirectories.begin(); it != Material::s_IncludeDirectories.end(); ++it) {
                std::string path = *it + '/' + SrcFile;
                std::replace(path.begin(), path.end(), '\\', '/');
                std::ifstream file(path, std::ios_base::binary | std::ios_base::ate);
                if (file) {
                    *pBytes = file.tellg();
                    *ppData = new char[*pBytes];
                    file.seekg(0, file.beg);
                    file.read((char*)*ppData, *pBytes);
                }
            }
        }
    };

    // Shader in pass
    class MetaShader
    {
    public:

        using VecDefs = ngfx::Vec<ShaderDefinition>;
        using VecVecDefs = ngfx::Vec<VecDefs>;

        using MultiComp = ngfx::Vec<String>;
        using VecMultiComp = ngfx::Vec<MultiComp>;

        explicit MetaShader(ShaderNode* shaderNode, SubShaderNode* subShader, PassNode* passNode);
        ~MetaShader() {}

        void ParsePragma(PragmaNode* pNode);
        void EnumerateVariants();

        void Compile(ngfx::Backend backend);

        friend class Material;
    private:
        void CompileEntry(ngfx::Backend backend);

    private:
        String                  m_Source;
        String                  m_File;
        String                  m_VertexEntry;
        String                  m_FragmentEntry;
        String                  m_GeometryEntry;

        // Subsets of multi_compile definitions
        VecVecDefs              m_CompDefs;

        VecMultiComp            m_MultiComps;
        ngfx::Vec<String>       m_CompileFeatures;
        ngfx::ShaderProfile     m_Profile;
        ngfx::VecUPtr<ShaderSnippet> m_IntShader;
    private:
        void Enumerate(int multiCompileIndex , VecVecDefs& result, VecDefs defines);
    };

    class MaterialPass
    {
    public:
        MaterialPass();
        ~MaterialPass();

        String                      Name;
        Material::RenderState       State;
        bool                        Compiled;
        ngfx::IBlob*                Library;
        ngfx::UniqPtr<MetaShader>   Shader;
    };

    MaterialPropertyInt::MaterialPropertyInt(int Value)
    {}

    MaterialPropertyInt::~MaterialPropertyInt()
    {
    }

    MaterialPropertyFloat::MaterialPropertyFloat(float Value)
    {
    }

    MaterialPropertyFloat::~MaterialPropertyFloat()
    {
    }

    MaterialPropertyVector4::MaterialPropertyVector4()
    {}

    MaterialPropertyVector4::~MaterialPropertyVector4()
    {
    }

    MaterialPropertyTexture::MaterialPropertyTexture()
    {}

    MaterialPropertyTexture::~MaterialPropertyTexture()
    {
    }

    Material::Material() : m_ActivePass(0), m_IsInRuntime(false), m_GfxBackend(ngfx::Backend::Vulkan)
    {
    }

    Material::~Material()
    {
    }

    void Material::ToggleKeyword(const char * Keyword, bool On)
    {
    }

    void Material::SetVector(const char * Name)
    {
    }

    void Material::SetFloat(const char * Name, float Value)
    {
    }

    void Material::SetInt(const char * Name, int Value)
    {
    }

    void Material::SetTexture(const char * Name)
    {
    }

    void Material::SetPass(int Index)
    {
        m_ActivePass = Index;
    }

    int Material::GetPassCount() const
    {
        return (int)m_Passes.num();
    }

    void Material::SetLOD(int Lod)
    {
    }

    bool Material::Compile()
    {
        m_Passes.iter_mut([this](ngfx::UniqPtr<MaterialPass> const& p) {
            p->Shader->Compile(m_GfxBackend);
        });

        return false;
    }

    bool Material::IsCompiled() const
    {
        return false;
    }

    Material* Material::Parse(String const& SLCode, String* error) {
        Error err;
        auto Toks = Tokenizer::Run(SLCode.c_str(), "__IN_MEMORY__", &err);
        if (err.HasError()) {
            if (error) {
                *error = err.PrintError();
            }
            return nullptr;
        }
        auto SNode = Parser::Parse(Toks, &err);
        if (err.HasError()) {
            if (error) {
                *error = err.PrintError();
            }
            return nullptr;
        }
        if (SNode && SNode->AsShader()) {
            return ((ShaderNode*)SNode.get())->CreateMaterial();
        }
        return nullptr;
    }

    Material* Material::ParseFile(const char * path, String* error)
    {
        std::ifstream MatFile(path);
        if (MatFile.bad()) {
            return nullptr;
        }
        Error err;
        std::string str((std::istreambuf_iterator<char>(MatFile)),
            std::istreambuf_iterator<char>());
        auto Toks = Tokenizer::Run(str.c_str(), path, &err);
        if (err.HasError()) {
            if (error) {
                *error = err.PrintError();
            }
            return nullptr;
        }
        auto SNode = Parser::Parse(Toks, &err);
        if (err.HasError()) {
            if (error) {
                *error = err.PrintError();
            }
            return nullptr;
        }
        if (SNode && SNode->AsShader()) {
            return ((ShaderNode*)SNode.get())->CreateMaterial();
        }
        return nullptr;
    }

    Material* Material::ParseAndCompileFile(const char * path, String* error)
    {
        Material* pMaterial = ParseFile(path, error);
        if (pMaterial) {
            pMaterial->Compile();
        }
        return pMaterial;
    }

    void Material::SetIncludeDirectories(ngfx::Vec<String> const & includeDir)
    {
        s_IncludeDirectories = includeDir;
    }

    // ShaderNode::CreateMaterial
    Material* ShaderNode::CreateMaterial()
    {
        std::string IncCode;
        if (m_IncludeCode) {
            CodeNode* incNode = static_cast<CodeNode*>(m_IncludeCode.get());
            IncCode = incNode->ToCode();
        }
        Material* RetMat = nullptr;
        if (!m_PropertyNodes.empty()) {
            m_PropertyNodes.iter([=](NodePtr const& p) {
                PropertyNode* pNode = static_cast<PropertyNode*>(p.get());
                pNode->m_ParsedProp;
            });
        }
        if (!m_SubShaderNode.empty()) {
            RetMat = new Material;
            RetMat->m_IncludeSource = IncCode;
            RetMat->m_Name = Name();
            SubShaderNode* subNode = static_cast<SubShaderNode*>(m_SubShaderNode.at(0).get());
            if (subNode->NumPasses() > 0) {
                subNode->IteratePass([=](PassNode* pNode) {
                    MaterialPass* pass = new MaterialPass;
                    pNode->OverrideRenderState(pass->State);
                    pass->Shader.reset(new MetaShader(this, subNode, pNode));
                    RetMat->m_Passes.push(std::move(ngfx::UniqPtr<MaterialPass>(pass)));
                });
            } else { // no sub passes but subshader has one
                MaterialPass* pass = new MaterialPass;
                subNode->OverrideRenderState(pass->State);
                pass->Shader.reset(new MetaShader(this, subNode, nullptr));
                RetMat->m_Passes.push(std::move(ngfx::UniqPtr<MaterialPass>(pass)));
            }
        }
        return RetMat;
    }

    MaterialPass::MaterialPass()
        : Shader(nullptr)
        , Library(nullptr)
        , Compiled(false)
    {}

    MaterialPass::~MaterialPass()
    {}

    MetaShader::MetaShader(ShaderNode* shaderNode, SubShaderNode* subShader, PassNode* passNode)
    {
        m_File = shaderNode->Path();
        CodeNode* incNode = nullptr;
        if (shaderNode) {
            incNode = shaderNode->GetInclude();
        }
        if (incNode) {
            if (incNode->HasPragma()) {
                incNode->IterPragma([=](PragmaNode* node) {
                    ParsePragma(node);
                });
            }
            m_Source += incNode->ToCode();
        }
        CodeNode* subShaderProgram = subShader->GetProgram();
        CodeNode* curPassProgram = nullptr;
        if (passNode) {
            curPassProgram = passNode->GetProgram();
        }
        if (curPassProgram) { // use pass program 
            if (curPassProgram->HasPragma()) {
                curPassProgram->IterPragma([=](PragmaNode* node) {
                    ParsePragma(node);
                });
            }
            m_Source += curPassProgram->ToCode();
        } else { // use subshader program
            if (subShaderProgram->HasPragma()) {
                subShaderProgram->IterPragma([=](PragmaNode* node) {
                    ParsePragma(node);
                });
            }
            m_Source += subShaderProgram->ToCode();
        }

    }
    void MetaShader::ParsePragma(PragmaNode* pNode)
    {
        switch (pNode->GetType())
        {
        case PragmaNode::PRAGMA_VERTEX_ENTRY:
            m_VertexEntry = pNode->GetEntry();
            break;
        case PragmaNode::PRAGMA_FRAGMENT_ENTRY:
            m_FragmentEntry = pNode->GetEntry();
            break;
        case PragmaNode::PRAGMA_GEOMETRY_ENTRY:
            m_GeometryEntry = pNode->GetEntry();
            break;
        case PragmaNode::PRAGMA_SHADER_TARGET:
            m_Profile = pNode->GetProfile();
            break;
        case PragmaNode::PRAGMA_MULTICOMPILE:
            m_MultiComps.push(pNode->GetMultiCompile());
            break;
        case PragmaNode::PRAGMA_SHADER_FEATURE:
            m_CompileFeatures.push(pNode->GetFeature());
            break;
        case PragmaNode::PRAGMA_ONLY_RENDERERS:
        case PragmaNode::PRAGMA_MULTICOMPILE_INSTANCING:
        case PragmaNode::PRAGMA_TESSELATE_ENTRY:
        default:
            break;
        }
    }
    
    void MetaShader::EnumerateVariants()
    {
        std::vector<bool> flags(m_MultiComps.num(), false);
        Enumerate(0, m_CompDefs, VecDefs());
    }

    void MetaShader::Enumerate(int sIndex, VecVecDefs& result, VecDefs defines)
    {
        if (sIndex == m_MultiComps.num())
        {
            result.push(defines);
            return;
        }
        auto const& comps = m_MultiComps.at(sIndex);
        for (int i = 0; i < comps.num(); i++)
        {
            defines.push({ comps.at(i).c_str() });
            Enumerate(sIndex + 1, result, defines);
            defines.pop();
        }
    }

    void MetaShader::Compile(ngfx::Backend backend)
    {
        if (!m_MultiComps.empty()) {
            EnumerateVariants();
            for (auto& defines : m_CompDefs)
            {
                defines.push({ nullptr, nullptr });
            }
        }
        CompileEntry(backend);
    }

    void MetaShader::CompileEntry(ngfx::Backend backend)
    {
        ShaderIncluder Includer;
        Ptr<IBlob> shader;
        Ptr<IBlob> message;

        if (m_VertexEntry.empty() || m_FragmentEntry.empty())
            return;

        struct EntrySh
        {
            const char* Entry;
            ShaderStage Stage;
        };

        EntrySh stages[] = {
            { m_VertexEntry.c_str(),    ShaderStage::Vertex },
            { m_FragmentEntry.c_str(),  ShaderStage::Pixel },
            { m_GeometryEntry.c_str(),  ShaderStage::Geometry },
        };

        for (auto cOpt : stages) { // every entry stage
            if (cOpt.Entry) {
                for (auto sd : m_CompDefs) // every variants
                {
                    /*ngfx::Compile(backend, m_Source.c_str(), m_File.c_str(), cOpt.Entry,
                        m_Profile, cOpt.Stage, ngfx::ShaderOptimizeLevel(0),
                        &Includer, &sd.at(0), shader.GetAddressOf(), message.GetAddressOf());*/
                }
            }
        }
    }

    bool Material::IsKeywordToggled(const char* Keyword) const
    {
        return true;
    }

    bool Material::IsFeatureEnabled(const char* Name) const
    {
        return false;
    }

    void Material::SetFeatureEnable(const char* Name, bool bEnable)
    {

    }

    void Material::CacheForRendering()
    {

    }

    void Material::CacheForCooking()
    {

    }
}