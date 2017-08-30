#pragma once
#include "shaderlab.h"
#include "sl_tokenizer.h"
#include <memory>
#include <unordered_map>
namespace shaderlab
{
    class Material;
    // //
    class CommentNode;
    
    // Shader "Name" BLOCK_NODE
    class ShaderNode;

    // Properties BLOCK_NODE
    // IDENTIFIER(LITERAL_STRING COMMA IDENTIFIER) ASSIGN (LITERAL|INTEGER|FLOAT) {}
    class PropertyNode;
    // Pass BLOCK_NODE
    class PassNode;
    // Tags BLOCK_NODE
    // LiteralString ASSIGN LiteralString
    class TagsNode;
    // SubShader BLOCK_NODE
    class SubShaderNode;

    // Fog { Fog Block }
    class FogNode;

    // Stencil {
    //  Ref 
    //  Pass 
    //  Fail
    //  ZFail
    //  Comp
    //  ReadMask
    //  WriteMask
    // }
    class StencilNode;

    // HLSLINCLUDE
    class ShaderIncludeNode;
    // has pragmas
    class ShaderProgramNode;

    class CodeNode;
    // ENDHLSL
    class ShaderEndNode;

    class EndNode;

    class ParserNode 
    {
    public:
        ParserNode() noexcept {}
        virtual ~ParserNode() noexcept {}

        virtual const CommentNode*          AsComment() const { return nullptr; }
        virtual const ShaderNode*           AsShader() const { return nullptr; }
        virtual const SubShaderNode*        AsSubShader() const { return nullptr; }
        virtual const PassNode*             AsPass() const { return nullptr; }
        virtual const TagsNode*             AsTags() const { return nullptr; }
        virtual const StencilNode*          AsStencil() const { return nullptr; }
        virtual const FogNode*              AsFog() const { return nullptr; }
        virtual const CodeNode*             AsCode() const { return nullptr; }
        virtual const ShaderProgramNode*    AsShaderProgram() const { return nullptr; }
        virtual const ShaderEndNode*        AsShaderEnd() const { return nullptr; }
    
    private:
        ParserNode(ParserNode const&) = delete;
        ParserNode& operator=(ParserNode const&) = delete;
    };

    using NodePtr = std::unique_ptr<ParserNode>;

    class CommentNode : public ParserNode
    {
    public:
        CommentNode(Token const& Comment);
        ~CommentNode() override;
        const CommentNode*  AsComment() const override;
        void SetComment(Token const& Comment) {
            m_Comment = Comment;
        }
    private:
        Token m_Comment;
    };

    class PragmaNode : public ParserNode
    {
    public:
        enum Type 
        {
            PRAGMA_VERTEX_ENTRY,
            PRAGMA_FRAGMENT_ENTRY,
            PRAGMA_GEOMETRY_ENTRY,
            PRAGMA_TESSELATE_ENTRY,
            PRAGMA_SHADER_TARGET, // #pragma target 3.0~6.0
            PRAGMA_MULTICOMPILE, // #pragma multi_compile _ XXX permutate all variants
            PRAGMA_SHADER_FEATURE, // #pragma shader_feature _ALPHATEST_ON toggle one variant
            PRAGMA_MULTICOMPILE_INSTANCING, // multi_compile_instancing
            PRAGMA_ONLY_RENDERERS, // #pragma only_renderers d3d11 ps4 xboxone vulkan metal switch
            PRAGMA_UNKNOWN,
        };

        explicit PragmaNode(Type inType = PRAGMA_UNKNOWN);
        ~PragmaNode() override;

        Type GetType() const { return m_Type; }
        void SetType(Type const& inType) { m_Type = inType; }

        const String& GetEntry() const {
            return m_Entry;
        }

        const String& GetFeature() const {
            return m_ShaderFeature;
        }

        const ngfx::Vec<String>& GetMultiCompile() const {
            return m_MultiCompileMacros;
        }

        ngfx::ShaderProfile GetProfile() const {
            return m_ShaderProfile;
        }

        const ngfx::Vec<String>& GetRenderers() const {
            return m_Renderers;
        }

        friend class PragmaParser;

    private:
        Type                    m_Type;
        String                  m_Entry;
        String                  m_ShaderFeature;
        ngfx::ShaderProfile     m_ShaderProfile;
        ngfx::Vec<String> m_Renderers;
        ngfx::Vec<String> m_MultiCompileMacros;
    };

    class CodeNode : public ParserNode
    {
    public:
        CodeNode();
        ~CodeNode() override;
        const CodeNode* AsCode() const override { return this; }
        std::string ToCode();
        void AppendCode(Token const& Tok)   { m_CodePieces.push_back(Tok); }
        void AppendPragma(NodePtr node)     { m_PragmaOptions.push_back(std::move(node)); }

        std::string GetEntryFunction(ngfx::ShaderStage shaderStage);

        bool HasPragma() const {
            return !m_PragmaOptions.empty();
        }

        template <typename IterFn>
        void IterPragma(IterFn fnIter) {
            for (NodePtr & node : m_PragmaOptions)
            {
                PragmaNode* pNode = static_cast<PragmaNode*>(node.get());
                if (pNode) {
                    fnIter(pNode);
                }
            }
        }

    private:
        std::vector<Token>      m_CodePieces;
        std::vector<NodePtr>    m_PragmaOptions;
    };

    // Shader
    class ShaderNode : public ParserNode
    {
    public:
        ShaderNode(const string_piece& Name = "");
        ~ShaderNode() override;
        
        const ShaderNode* AsShader() const override { return this; }

        void AddSubShader(NodePtr node) {
            m_SubShaderNode.push(std::move(node));
        }

        void SetIncludeSource(NodePtr node) {
            m_IncludeCode = std::move(node);
        }

        void AddProperty(NodePtr node) {
            m_PropertyNodes.push(std::move(node));
        }

        void SetName(const string_piece& Name) {
            m_Name = Name.to_string();
        }
        const String& Name() const { return m_Name; }

        void SetFilePath(String const& Path) {
            m_FilePath = Path;
        }
        const String& Path() const { return m_FilePath; }

        CodeNode* GetInclude() const {
            return static_cast<CodeNode*>(m_IncludeCode.get());
        }

        Material* CreateMaterial();

    private:
        String                  m_Name; // Shader "Name";
        String                  m_FilePath;
        ngfx::Vec<NodePtr>      m_PropertyNodes;
        ngfx::Vec<NodePtr>      m_SubShaderNode;
        NodePtr                 m_IncludeCode;
        ngfx::Vec<NodePtr>      m_Comments;
    };

    // Pass
    class PassNode : public ParserNode
    {
    public:
        PassNode();
        ~PassNode() override;
        
        const PassNode* AsPass() const override { return this; }
        
        void SetCode(NodePtr node) {
            m_CodeNode = std::move(node);
        }
        
        void SetName(String const& Name) {
            m_Name = Name;
        }
        
        const String& Name() const { return m_Name; }
        
        const Material::RenderState& RenderState() const { return m_State; }
        
        void SetTags(std::unique_ptr<TagsNode> node) {
            m_TagNode = std::move(node);
        }
        
        int GetLOD() const { return m_LOD; }
        
        const String& GetTagValue(String const& TagKey) const;
        
        bool HasCode() const { return m_CodeNode != nullptr; }
        
        String PrintCode();
        
        CodeNode* GetProgram() const {
            return static_cast<CodeNode*>(m_CodeNode.get());
        }
        
        void OverrideRenderState(Material::RenderState& inState);
        
        friend class Parser;
    private:
        String                      m_Name;
        String                      m_DefaultTag;
        NodePtr                     m_CodeNode;
        Material::RenderState       m_State;
        int                         m_LOD;
        std::unique_ptr<TagsNode>   m_TagNode;
    };

    // SubShader
    class SubShaderNode : public ParserNode
    {
    public:
        SubShaderNode();
        ~SubShaderNode() override;

        const SubShaderNode* AsSubShader() const override { return this; }

        void AddPass(NodePtr node) {
            m_PassNodes.push_back(std::move(node));
        }

        void SetProgram(NodePtr program) {
            m_Program = std::move(program);
        }

        void SetTags(std::unique_ptr<TagsNode> node) {
            m_TagNode = std::move(node);
        }

        int NumPasses() const {
            return (int)m_PassNodes.size();
        }

        int GetLOD() const { return m_LOD; }

        const String& GetTagValue(String const& TagKey) const;

        const Material::RenderState& GetRenderState() const {
            return m_State;
        }

        void OverrideRenderState(Material::RenderState& inState);

        template <typename TPassIterFn>
        void IteratePass(TPassIterFn iPassFn)
        {
            for (auto& pNode : m_PassNodes)
            {
                PassNode* passNode = static_cast<PassNode*> (pNode.get());
                iPassFn(passNode);
            }
        }

        CodeNode* GetProgram() const {
            return static_cast<CodeNode*>(m_Program.get());
        }

        friend class Parser;
    private:
        std::vector<NodePtr>        m_PassNodes;
        Material::RenderState       m_State;
        std::unique_ptr<TagsNode>   m_TagNode;
        String                      m_DefaultTag;
        int                         m_LOD;
        NodePtr                     m_Program;
    };
    
    class PropertyNode : public ParserNode 
    {
    public:
        PropertyNode();
        ~PropertyNode() override;

        void SetName(String const& Name) {
            m_Name = Name;
        }
        void SetDisplayName(String const& DisplayName) {
            m_DisplayName = DisplayName;
        }
        void SetAnnotation(String const& Annot) {
            m_Annotation = Annot;
        }

        void Construct(Token::Type const& Type);

        void SetTextureName(String const& Name);
        void SetFloat(double Val);
        void SetFloatWithRange(double Val, double Min, double Max);
        void SetInt(int Val);
        void SetIntWithRange(int Val, int Min, int Max);

        MaterialProperty::Type GetType() const;
        
        friend class ShaderNode;
    private:
        String m_Name;
        String m_DisplayName;
        String m_Annotation;
        std::unique_ptr<MaterialProperty> m_ParsedProp;
    };

    class StencilNode : public ParserNode 
    {
    public:
        StencilNode();
        ~StencilNode() override;
        virtual const StencilNode*  AsStencil() const { return this; }

        friend class Parser;
    private:
        ngfx::ComparisonFunction    m_Comp;
        ngfx::StencilOperation      m_Pass;
        ngfx::StencilOperation      m_Fail;
        ngfx::StencilOperation      m_ZFail;
        uint32_t                    m_ReadMask;
        uint32_t                    m_WriteMask;
        uint32_t                    m_Ref;
    };

    class TagsNode : public ParserNode
    {
    public:
        TagsNode();
        ~TagsNode() override;
        virtual const TagsNode* AsTags() const { return this; }
        const String& GetTag(String const& Key) const;

        friend class Parser;
    private:
        std::unordered_map<String, String> m_Tags;
        String m_Default;
    };

    class FogNode : public ParserNode
    {
    public:
        virtual const FogNode*  AsFog() const { return this; }
    };

    class EndNode : public ParserNode {
    public:
        EndNode(const Token& Tok);
        ~EndNode() override;

    private:
        Token m_Tok;
    };

    class Parser;
    typedef std::unique_ptr<ParserNode>(Parser::*FnPrefix)(const Token& token);
    typedef std::unique_ptr<ParserNode>(Parser::*FnInfix)(std::unique_ptr<ParserNode> left, const Token& token);

    class BaseParser
    {
    protected:
        const std::vector<Token>&   m_Tokens;
        int                         m_Offset = 0;
        Token                       m_Invalid;
    public:
        BaseParser(const std::vector<Token>& Toks) : m_Tokens(Toks), m_Offset(0) {}
        virtual ~BaseParser() {}
    protected:
        virtual bool    HasError() const { return false; }
        bool            AtEnd() const { return m_Offset == m_Tokens.size(); }
        bool            Done() const { return AtEnd() || HasError(); }
        const Token&    Cur() const { return m_Tokens[m_Offset]; }
        const Token&    Consume() { return m_Tokens[m_Offset++]; }
        bool            Match(Token::Type const& Type);
        bool            LookAhead(Token::Type const& Type);
    };

    class Parser : public BaseParser
    {
    public:
        static NodePtr Parse(const std::vector<Token>& toks, Error* Err = nullptr);

    private:
        Parser(const std::vector<Token>& Toks, Error* Err = nullptr);
        NodePtr Run();
        
        NodePtr Shader(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr ShaderBlock(std::unique_ptr<ParserNode> left, const Token& cur);

        NodePtr SubShader(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr SubShaderBlock(std::unique_ptr<ParserNode> left, const Token& cur);

        NodePtr Tags(std::unique_ptr<ParserNode> left, const Token& cur);
        std::unique_ptr<TagsNode> TagsBlock();
        
        NodePtr Pass(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr PassBlock(std::unique_ptr<ParserNode> left, const Token& cur);
        NodePtr Name(std::unique_ptr<ParserNode> left, const Token& tok);
        
        // code
        NodePtr HLSLInclude(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr HLSLProgram(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr HLSL();
        NodePtr Pragma(std::unique_ptr<ParserNode> left, const Token& tok);

        NodePtr Properties(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr Property(std::unique_ptr<ParserNode> left, const Token& tok);

        NodePtr PropertyRange(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr PropertyInt(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr PropertyFloat(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr PropertyVector(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr PropertyTexture(std::unique_ptr<ParserNode> left, const Token& tok);

        NodePtr Fallback(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr Comment(std::unique_ptr<ParserNode> left, const Token& tok);

        NodePtr LOD(std::unique_ptr<ParserNode> left, const Token& tok);

        NodePtr Blend(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr BlendOp(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr AlphaToMask(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr AlphaTest(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr ColorMask(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr Fog(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr Cull(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr ZTest(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr ZWrite(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr ZClip(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr Offset(std::unique_ptr<ParserNode> left, const Token& tok);

        NodePtr Stencil(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr StencilBlock();
        NodePtr StencilWriteMask(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr StencilReadMask(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr StencilRef(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr StencilComp(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr StencilPass(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr StencilFail(std::unique_ptr<ParserNode> left, const Token& tok);
        NodePtr StencilZFail(std::unique_ptr<ParserNode> left, const Token& tok);

        static std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > s_RootExpr;
        static std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > s_ShaderExpr;
        static std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > s_PropertiesExpr;
        static std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > s_PropertyExpr;
        static std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > s_SubShaderExpr;
        static std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > s_PassExpr;
        static std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > s_StencilExpr;
        static std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > s_Expr;

    private:
        bool    ParseBlendFactor(ngfx::RenderTargetBlendState& src);
        bool    ParseBlendFactor(ngfx::BlendFactor& src, ngfx::BlendFactor& dst);
        bool    ParseBlendOperation(ngfx::BlendOperation& color, ngfx::BlendOperation& alpha);
        ngfx::DepthStencilState*    ExtractDepthStencil(ParserNode* left);
        ngfx::BlendState*           ExtractBlend(ParserNode* left);
        ngfx::RasterizerState*      ExtractRaster(ParserNode* left);
    private:
        bool HasError() const override { return m_Err && m_Err->HasError(); }
        const Token& Recede();
        bool ConsumeNumber(Token& Tok);
        void SpawnError(String const& Err, String const& Help = "");
    private:
        static bool IsNumber(Token const& Tok);
        static bool IsProperty(Token const& Tok);
        static bool CouldBeIdentifier(Token const& Tok);
    private:
        Error*              m_Err;
        std::vector<Error>  m_PendingErrors;
    };
}
