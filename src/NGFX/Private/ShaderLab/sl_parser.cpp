#include "sl_parser.h"
#include "shaderlab_private.h"

#include <sstream>

namespace shaderlab
{
    //@see https://docs.unity3d.com/Manual/SL-Pass.html
    std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > Parser::s_Expr = {
    // Source Code
    { Token::TOK_PRAGMA,            { nullptr, &Parser::Pragma } },
    };

    std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > Parser::s_ShaderExpr = {
    { Token::TOK_COMMENT,           { nullptr, &Parser::Comment } },
    { Token::TOK_PROPERTIES,        { nullptr, &Parser::Properties } },
    { Token::TOK_SUBSHADER,         { nullptr, &Parser::SubShader } },
    { Token::TOK_FALLBACK,          { nullptr, &Parser::Fallback } },
    { Token::TOK_HLSLINCLUDE,       { nullptr, &Parser::HLSLInclude } },
    };

    std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > Parser::s_SubShaderExpr = {
    { Token::TOK_COMMENT,           { nullptr, &Parser::Comment } },
    { Token::TOK_HLSLPROG,          { nullptr, &Parser::HLSLProgram } }, // HLSLPROGRAM
    { Token::TOK_PASS,              { nullptr, &Parser::Pass } }, // Pass
    { Token::TOK_TAGS,              { nullptr, &Parser::Tags } },
    { Token::TOK_LOD,               { nullptr, &Parser::LOD } },
    // Render State
    { Token::TOK_STATE_BLEND,       { nullptr, &Parser::Blend } },
    { Token::TOK_STATE_BLENDOP,     { nullptr, &Parser::BlendOp } },
    { Token::TOK_STATE_ALPHATOMASK, { nullptr, &Parser::AlphaToMask } },
    { Token::TOK_STATE_FOG,         { nullptr, &Parser::Fog } },
    { Token::TOK_STATE_ALPHATEST,   { nullptr, &Parser::AlphaTest } },
    { Token::TOK_STATE_CULL,        { nullptr, &Parser::Cull } },
    { Token::TOK_STATE_ZTEST,       { nullptr, &Parser::ZTest } },
    { Token::TOK_STATE_ZWRITE,      { nullptr, &Parser::ZWrite } },
    { Token::TOK_STATE_OFFSET,      { nullptr, &Parser::Offset } },
    { Token::TOK_STATE_COLORMASK,   { nullptr, &Parser::ColorMask } },
    { Token::TOK_STATE_STENCIL,     { nullptr, &Parser::Stencil } },
    };

    std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > Parser::s_PassExpr = {
    { Token::TOK_COMMENT,           { nullptr, &Parser::Comment } },
    { Token::TOK_HLSLPROG,          { nullptr, &Parser::HLSLProgram } }, // HLSLPROGRAM
    { Token::TOK_PASS_NAME,         { nullptr, &Parser::Name } },
    { Token::TOK_TAGS,              { nullptr, &Parser::Tags } },
    { Token::TOK_LOD,               { nullptr, &Parser::LOD } },
    // Render State
    { Token::TOK_STATE_BLEND,       { nullptr, &Parser::Blend } },
    { Token::TOK_STATE_BLENDOP,     { nullptr, &Parser::BlendOp } },
    { Token::TOK_STATE_ALPHATOMASK, { nullptr, &Parser::AlphaToMask } },
    { Token::TOK_STATE_FOG,         { nullptr, &Parser::Fog } },
    { Token::TOK_STATE_ALPHATEST,   { nullptr, &Parser::AlphaTest } },
    { Token::TOK_STATE_CULL,        { nullptr, &Parser::Cull } },
    { Token::TOK_STATE_ZTEST,       { nullptr, &Parser::ZTest } },
    { Token::TOK_STATE_ZWRITE,      { nullptr, &Parser::ZWrite } },
    { Token::TOK_STATE_OFFSET,      { nullptr, &Parser::Offset } },
    { Token::TOK_STATE_COLORMASK,   { nullptr, &Parser::ColorMask } },
    { Token::TOK_STATE_STENCIL,     { nullptr, &Parser::Stencil } },
    };

    bool Parser::IsProperty(Token const& Tok) {
        return Tok.GetType() >= Token::TOK_PROP_SAMPLER_1D && Tok.GetType() <= Token::TOK_PROP_VECTOR;
    }

    bool Parser::CouldBeIdentifier(Token const& Tok) {
        return (Tok.GetType() >= Token::TOK_PROP_SAMPLER_CUBE && Tok.GetType() <= Token::TOK_PROP_VECTOR)
            || IsKeyword(Tok.Str());
    }

    bool Parser::IsNumber(Token const& Tok) {
        return Tok.GetType() == Token::TOK_INTEGER || Tok.GetType() == Token::TOK_FLOAT;
    }

    NodePtr Parser::Parse(const std::vector<Token>& toks, Error* Err) {
        Parser p(toks, Err);
        return p.Run();
    }

    Parser::Parser(const std::vector<Token>& toks, Error* Err)
        : BaseParser(toks), m_Err(Err)
    {}

    NodePtr Parser::Run()
    {
        NodePtr ret;
        std::unique_ptr< ShaderNode > shaderNode = std::make_unique<ShaderNode>();
        if (!m_Tokens.empty()) {
            shaderNode->SetFilePath((String)m_Tokens.front().GetLocation().Path);
        }
        while (!AtEnd()) {
            if (LookAhead(Token::TOK_SHADER)) {
                ret = Shader(std::move(shaderNode), Consume());
            } else if (LookAhead(Token::TOK_COMMENT)) {
                Consume();
            } else {
                if(!HasError())
                    SpawnError("Parsing Shader error occured", "Unexpected token, only COMMENTS or Shader accepted.");
                break;
            }
        }
        return ret;
    }
    // Shader "Name" { SUBSHADER | FALLBACK | HLSLINCLUDE | COMMENT | PROPERTIES }
    NodePtr Parser::Shader(std::unique_ptr<ParserNode> left, const Token & tok) {
        auto Name = Cur();
        if (Match(Token::TOK_LITERAL_STRING)) {
            ShaderNode* shaderNode = static_cast<ShaderNode*>(left.get());
            shaderNode->SetName(Unquote(Name.Str()));
            if (Match(Token::TOK_LEFT_BRACE)) {
                left = ShaderBlock(std::move(left), Cur());
            } else {
                SpawnError("Only '{' is allowed to follow 'Shader \"NAME\" ' ", "");
            }
            return left;
        } else {
            SpawnError("Only literal string is allowed to follow 'Shader'", "");
            return NodePtr();
        }
    }
    // SUBSHADER | FALLBACK | HLSLINCLUDE | COMMENT | PROPERTIES }
    NodePtr Parser::ShaderBlock(std::unique_ptr<ParserNode> left, const Token & cur)
    {
        while (!AtEnd()) {
            if (Match(Token::TOK_RIGHT_BRACE)) { // Shader End
                // End
                break;
            }
            auto Next = Consume();
            FnInfix infix = s_ShaderExpr[Next.GetType()].second;
            if (infix == nullptr) {
                // SpawnError()
                return NodePtr();
            }
            left = (this->*infix)(std::move(left), Next); // Left Should be ShaderNode
            if (AtEnd()) {
                SpawnError("Unexpected EoF in Shader block", "'}' expected");
            }
            if (HasError()) {
                return NodePtr();
            }
        }
        return left;
    }

    NodePtr Parser::Properties(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        if (!Match(Token::TOK_LEFT_BRACE)) {
            SpawnError("Unable to parse PROPERTIES block", "Expected '{' here");
            return nullptr;
        }
        while (!AtEnd()) {
            if (Match(Token::TOK_RIGHT_BRACE)) { // Property End
                break;
            }
            auto Next = Consume();
            FnInfix infix = s_PropertiesExpr[Next.GetType()].second;
            if (infix == nullptr) {
                SpawnError("Unable to parse this property");
                return NodePtr();
            }
            left = (this->*infix)(std::move(left), Next); // left is ShaderNode
            if (HasError()) {
                return NodePtr();
            }
        }
        return left;
    }
    // SubShader { PASS | TAG | STATE | HLSLPROGRAM }
    NodePtr Parser::SubShader(std::unique_ptr<ParserNode> left, const Token & tok) // cur token is '{'
    {
        if (!Match(Token::TOK_LEFT_BRACE)) {
            SpawnError("Unable to parse SUBSHADER block", "Expected '{' here");
            return nullptr;
        }
        ShaderNode* sNode = static_cast<ShaderNode*>(left.get());
        if (!sNode) {
            return NodePtr();
        }    
        left = SubShaderBlock(std::move(left), Consume()); // ret should be ShaderNode

        return std::move(left);
    }
    // PASS | TAG | STATE | HLSLPROGRAM }
    NodePtr Parser::SubShaderBlock(std::unique_ptr<ParserNode> left, const Token& cur)
    {
        ShaderNode* sNode = static_cast<ShaderNode*>(left.get());
        if (!sNode) {
            return NodePtr();
        }
        Recede();
        NodePtr subShaderNode = std::make_unique<SubShaderNode>();
        while (!AtEnd()) {
            if (Match(Token::TOK_RIGHT_BRACE)) { // SubShader End
                break;
            }
            auto Next = Consume();
            FnInfix infix = s_SubShaderExpr[Next.GetType()].second;
            if (infix) {
                subShaderNode = (this->*infix)(std::move(subShaderNode), Next);
            } else {
                SpawnError("Unaccepted token in SUBSHADER block");
                break;
            }
            if (AtEnd()) {
                SpawnError("Unexpected EoF in SubShader block", "'}' expected");
                break;
            }
            if (HasError()) {
                return NodePtr();
            }
        }
        sNode->AddSubShader(std::move(subShaderNode));
        return left; // return shadernode
    }
    NodePtr Parser::Tags(std::unique_ptr<ParserNode> left, const Token & cur)
    {
        if (left->AsSubShader()) {
            auto node = TagsBlock();
            SubShaderNode* subNode = static_cast<SubShaderNode*>(left.get());
            subNode->SetTags(std::move(node));
        } else if (left->AsPass()) {
            auto node = TagsBlock();
            PassNode* passNode = static_cast<PassNode*>(left.get());
            passNode->SetTags(std::move(node));
        } else {
            return nullptr;
        }

        return left;
    }
    std::unique_ptr<TagsNode> Parser::TagsBlock()
    {
        if (!Match(Token::TOK_LEFT_BRACE)) {
            SpawnError("Parsing Tags node failed", "Only '{' can follow after Tags");
            return nullptr;
        }
        auto tagNode = std::make_unique<TagsNode>();
        while (!AtEnd()) {
            if (LookAhead(Token::TOK_RIGHT_BRACE)) {
                Consume();
                break;
            }
            if (LookAhead(Token::TOK_LITERAL_STRING)) {
                auto tagKey = (std::string)Consume().Str();
                if (!Match(Token::TOK_ASSIGN)) {
                    SpawnError("Parsing Tags Node failed", "Expected '=' here");
                    return nullptr;
                }
                auto tagValue = (std::string)Cur().Str();
                if (!Match(Token::TOK_LITERAL_STRING)) {
                    SpawnError("Parsing Tags Node value failed", "Expected \"LITERAL_STRING\" here");
                    return nullptr;
                }
                tagNode->m_Tags[tagKey] = tagValue;
            } else {
                SpawnError("Parsing Tags Node key failed", "Expected \"LITERAL_STRING\" here");
                return nullptr;
            }
            if (AtEnd()) {
                SpawnError("Unexpected EoF in Tag block", "'}' expected");
                break;
            }
        }
        return tagNode;
    }
    // PASS { Name | TAG | HLSLPROGRAM | STATE }
    NodePtr Parser::Pass(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        SubShaderNode* sNode = static_cast<SubShaderNode*>(left.get());
        if (sNode) {
            left = PassBlock(std::move(left), Consume());
        }
        return left;
    }
    // Name | TAG | HLSLPROGRAM | STATE }
    NodePtr Parser::PassBlock(std::unique_ptr<ParserNode> left, const Token & cur)
    {
        SubShaderNode* subNode = static_cast<SubShaderNode*>(left.get());
        if (!subNode) {
            return NodePtr();
        }
        NodePtr passNode = std::make_unique<PassNode>();
        while (!AtEnd()) {
            if (Match(Token::TOK_RIGHT_BRACE)) { // SubShader End
                break;
            }
            auto Next = Consume();
            FnInfix infix = s_PassExpr[Next.GetType()].second;
            if (infix) {
                passNode = (this->*infix)(std::move(passNode), Next);
            } else {
                SpawnError("Unaccepted Token in PASS block");
                break;
            }
            if (AtEnd()) {
                SpawnError("Unexpected EoF in Pass block", "'}' expected");
                break;
            }
        }
        subNode->AddPass(std::move(passNode));
        return left; // return subshadernode
    }

    NodePtr Parser::Name(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        auto CurTok = Cur();
        if (!Match(Token::TOK_LITERAL_STRING)) {
            SpawnError("Following Token should be lteral string !");
            return NodePtr();
        }
        PassNode* pNode = static_cast<PassNode*>(left.get());
        if (!pNode) {
            SpawnError("Name is only accepted by 'Pass'");
            return NodePtr();
        }
        pNode->SetName(CurTok.Str().to_string());
        return std::move(left);
    }
    
    // HLSLINCLUDE ... ENDHLSL
    NodePtr Parser::HLSLInclude(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        NodePtr hlslNode = HLSL();
        ShaderNode* shaderNode = static_cast<ShaderNode*>(left.get());
        // left should be shadernode
        shaderNode->SetIncludeSource(std::move(hlslNode));
        return left;
    }
    NodePtr Parser::HLSLProgram(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        // left could be subshader or pass
        NodePtr hlslNode = HLSL();
        if (left->AsSubShader()) {
            SubShaderNode* subShaderNode = static_cast<SubShaderNode*>(left.get());
            subShaderNode->SetProgram(std::move(hlslNode));
        } else if (left->AsPass()) {
            PassNode* passNode = static_cast<PassNode*>(left.get());
            passNode->SetCode(std::move(hlslNode));
        } else {
            SpawnError("HLSLPROGRAM is only accepted in PASS an SUBSHADER");
            return NodePtr();
        }
        return left;
    }
    NodePtr Parser::HLSL()
    {
        NodePtr includeNode = std::make_unique<CodeNode>();
        CodeNode* cNode = static_cast<CodeNode*>(includeNode.get());
        
        while (!AtEnd()) {
            if (Match(Token::TOK_HLSLEND)) {
                break;
            }
            auto Tok = Consume();
            if (Tok.GetType() == Token::TOK_PRAGMA) {
                //NodePtr pragmaNode = std::make_unique<PragmaNode>();
                includeNode = Pragma(std::move(includeNode), Tok);
            } else if (Tok.GetType() == Token::TOK_SOURCE) {
                cNode->AppendCode(Tok);
            } else {
                SpawnError("Unaccepted token in HLSLINCLUDE block", "Only #pragma or HLSL source is accepted");
                break;
            }
            if (AtEnd()) {
                SpawnError("Unexpected EoF in HLSL block", "ENDHLSL expected");
            }
            if (HasError())
                return nullptr;
        }
        return includeNode;
    }
    NodePtr Parser::Fallback(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        if (!Match(Token::TOK_IDENTIFIER) && !Match(Token::TOK_LITERAL_STRING))
        {
            SpawnError("Unable to parse Fallback", "\"LITERAL_STRING\" or Off expected here");
            return nullptr;
        }
        return left;
    }
    
    NodePtr Parser::Comment(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        /*if (tok.GetType() == Token::TOK_COMMENT)
        {
            Recede();
        }
        Consume();*/
        return left;
    }

    NodePtr Parser::LOD(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        auto curTok = Cur();
        if (!Match(Token::TOK_INTEGER)) {
            SpawnError("Parsing LOD failed", "INTEGER expected here");
            return left;
        }
        if (left->AsPass()) {
            PassNode* passNode = static_cast<PassNode*>(left.get());
            passNode->m_LOD = curTok.Int();
        } else if (left->AsSubShader()) {
            SubShaderNode* subShaderNode = static_cast<SubShaderNode*>(left.get());
            subShaderNode->m_LOD = curTok.Int();
        }
        return left;
    }

    const Token& Parser::Recede()
    {
        if (m_Offset >= 1) {
            m_Offset--;
        }
        return m_Tokens[m_Offset];
    }

    bool Parser::ConsumeNumber(Token& Tok)
    {
        if (LookAhead(Token::TOK_INTEGER) || LookAhead(Token::TOK_FLOAT)) {
            Tok = Consume();
            return true;
        }
        return false;
    }
    void Parser::SpawnError(String const& Err, String const & Help)
    {
        if (m_Err) {
            *m_Err = Error(AtEnd() ? m_Tokens.back().GetLocation() : Cur().GetLocation(), 
                Err, Help);
        } else {
            m_Err = new Error(AtEnd() ? m_Tokens.back().GetLocation() : Cur().GetLocation(), 
                Err, Help);
        }
    }
    
    CommentNode::CommentNode(Token const & Comment)
    {
    }

    CommentNode::~CommentNode()
    {
    }
    
    const CommentNode* CommentNode::AsComment() const
    {
        return this;
    }
    
    CodeNode::CodeNode()
    {
    }
    
    CodeNode::~CodeNode()
    {
    }
    
    std::string CodeNode::ToCode()
    {
        String code;
        for (auto CodeLine : m_CodePieces) {
            int line = CodeLine.GetLocation().Line;
            std::ostringstream oss;
            oss << (std::string)CodeLine.Str() << "\n" << "#line " << line << " \"" << (std::string)CodeLine.GetLocation().Path << "\"" << "\n";
            code += oss.str();
        }
        return code;
    }

    std::string CodeNode::GetEntryFunction(ngfx::ShaderStage shaderStage)
    {
        switch (shaderStage)
        {
        case ngfx::ShaderStage::Vertex:
        case ngfx::ShaderStage::Pixel:
        case ngfx::ShaderStage::Geometry:
        case ngfx::ShaderStage::Domain:
        case ngfx::ShaderStage::Hull:
        default:
            break;
        }
        return "";
    }

    EndNode::EndNode(const Token & Tok)
        : m_Tok(Tok)
    {
    }
    
    EndNode::~EndNode()
    {
    }
    
    ShaderNode::ShaderNode(const string_piece& Name)
        : m_Name(Name.to_string())
    {
    }
    ShaderNode::~ShaderNode()
    {
    }
    SubShaderNode::SubShaderNode()
    {
    }
    SubShaderNode::~SubShaderNode()
    {
    }
    const String & SubShaderNode::GetTagValue(String const & TagKey) const {
        if (m_TagNode) {
            return m_TagNode->GetTag(TagKey);
        }
        return m_DefaultTag;
    }
    void SubShaderNode::OverrideRenderState(Material::RenderState & inState)
    {
        inState = m_State;
    }
    PassNode::PassNode()
    {
    }
    PassNode::~PassNode()
    {
    }
    const String & PassNode::GetTagValue(String const & TagKey) const {
        if (m_TagNode) {
            return m_TagNode->GetTag(TagKey);
        }
        return m_DefaultTag;
    }
    String PassNode::PrintCode()
    {
        if (m_CodeNode) {
            CodeNode* cNode = static_cast<CodeNode*>(m_CodeNode.get());
            return cNode->ToCode();
        }
        return String();
    }
    void PassNode::OverrideRenderState(Material::RenderState& inState)
    {
        inState = m_State;
    }
    PropertyNode::PropertyNode()
    {
    } 
    PropertyNode::~PropertyNode()
    {
    }
    void PropertyNode::Construct(Token::Type const& Type)
    {
        switch (Type) {
        case Token::TOK_PROP_INTEGER:
            m_ParsedProp = std::make_unique<MaterialPropertyInt>();
            break;
        case Token::TOK_PROP_FLOAT:
            m_ParsedProp = std::make_unique<MaterialPropertyFloat>();
            break;
        case Token::TOK_PROP_COLOR:
            m_ParsedProp = std::make_unique<MaterialPropertyVector4>();
            break;
        case Token::TOK_PROP_VECTOR:
            m_ParsedProp = std::make_unique<MaterialPropertyVector4>();
            break;
        case Token::TOK_PROP_RANGE:
            m_ParsedProp = std::make_unique<MaterialPropertyFloat>();
            break;
        case Token::TOK_PROP_SAMPLER_1D:
        case Token::TOK_PROP_SAMPLER_2D:
        case Token::TOK_PROP_SAMPLER_3D:
        case Token::TOK_PROP_SAMPLER_CUBE:
            m_ParsedProp = std::make_unique<MaterialPropertyTexture>();
            break;
        }
        if (m_ParsedProp)
        {
            m_ParsedProp->m_Name = m_Name;
            m_ParsedProp->m_DisplayName = m_DisplayName;
            m_ParsedProp->m_Annotation = m_Annotation;
        }
    }
    void PropertyNode::SetTextureName(String const & Name)
    {
        if (m_ParsedProp) {
            MaterialPropertyTexture* Texture = static_cast<MaterialPropertyTexture*>(m_ParsedProp.get());
            if (Texture) {
                Texture->SetTextureName(Name);
            }
        }
    }
    void PropertyNode::SetFloat(double Val)
    {
        if (m_ParsedProp) {
            MaterialPropertyFloat* Float = static_cast<MaterialPropertyFloat*>(m_ParsedProp.get());
            if (Float) {
                Float->SetValue(Val);
            }
        }
    }
    void PropertyNode::SetFloatWithRange(double Val, double Min, double Max)
    {
        if (m_ParsedProp) {
            MaterialPropertyFloat* Float = static_cast<MaterialPropertyFloat*>(m_ParsedProp.get());
            if (Float) {
                Float->SetValue(Val);
                Float->SetRange(Min, Max);
            }
        }
    }
    void PropertyNode::SetInt(int Val)
    {
        if (m_ParsedProp) {
            MaterialPropertyInt* Int = static_cast<MaterialPropertyInt*>(m_ParsedProp.get());
            if (Int) {
                Int->SetValue(Val);
            }
        }
    }
    void PropertyNode::SetIntWithRange(int Val, int Min, int Max)
    {
        if (m_ParsedProp) {
            MaterialPropertyInt* Int = static_cast<MaterialPropertyInt*>(m_ParsedProp.get());
            if (Int) {
                Int->SetValue(Val);
                Int->SetRange(Min, Max);
            }
        }
    }
    MaterialProperty::Type PropertyNode::GetType() const
    {
        return MaterialProperty::Type();
    }

    TagsNode::TagsNode()
    {
    }
    TagsNode::~TagsNode()
    {
    }
    
    const String& TagsNode::GetTag(String const & Key) const
    {
        if (m_Tags.find(Key) != m_Tags.end()) {
            return m_Tags.at(Key);
        }
        return m_Default;
    }
    PragmaNode::PragmaNode(Type inType) : m_Type(inType), m_ShaderProfile(ngfx::ShaderProfile::SM5)
    {
    }
    PragmaNode::~PragmaNode()
    {
    }
    StencilNode::StencilNode()
    {
    }
    StencilNode::~StencilNode()
    {
    }

    bool BaseParser::Match(Token::Type const & Type)
    {
        if (!LookAhead(Type))
            return false;
        Consume();
        return true;
    }

    bool BaseParser::LookAhead(Token::Type const & Type)
    {
        if (AtEnd())
            return false;
        return Cur().GetType() == Type;
    }

}