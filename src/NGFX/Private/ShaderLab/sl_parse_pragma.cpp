#include "sl_parser.h"

namespace shaderlab
{
    class PragmaParser : public BaseParser
    {
    public:
        enum ErrorCode
        {
            E_SUCCESS,
            E_UNSUPPORTED_PRAGMA,
            E_UNKNOWN_PRAGMA,
            E_UNEXPECTED_TOKEN,
        };

        PragmaParser(Parser* inParser, CodeNode* inCodeNode, const std::vector<Token>& Toks);
        ~PragmaParser();

        ErrorCode DoParse();
        std::unique_ptr<PragmaNode> VertexFunction(Token const& tok);
        std::unique_ptr<PragmaNode> FragmentFunction(Token const& tok);
        std::unique_ptr<PragmaNode> GeometryFunction(Token const& tok);
        std::unique_ptr<PragmaNode> TesselateFunction(Token const& tok);
        std::unique_ptr<PragmaNode> ShaderFeature(Token const& tok);
        std::unique_ptr<PragmaNode> MultiCompile(Token const& tok);
        std::unique_ptr<PragmaNode> ShaderTarget(Token const& tok);
        std::unique_ptr<PragmaNode> OnlyRenderers(Token const& tok);
    private:
        Parser*             m_Parent;
        CodeNode*           m_CodeNode; // mut ref
        ErrorCode           m_Error;
    };
    typedef std::unique_ptr<PragmaNode>(PragmaParser::*FnPPrefix)(const Token& token);

    std::unordered_map<string_piece, FnPPrefix> s_Pragmas = {
    { "vertex",         &PragmaParser::VertexFunction },
    { "fragment",       &PragmaParser::FragmentFunction },
    { "geometry",       &PragmaParser::GeometryFunction },
    { "tesselate",      &PragmaParser::TesselateFunction },
    { "domain",         &PragmaParser::TesselateFunction },
    { "hull",           &PragmaParser::TesselateFunction },
    { "shader_feature", &PragmaParser::ShaderFeature },
    { "multi_compile",  &PragmaParser::MultiCompile },
    { "only_renderers", &PragmaParser::OnlyRenderers },
    { "target",         &PragmaParser::ShaderTarget },
    };

    NodePtr Parser::Pragma(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        CodeNode* cNode = static_cast<CodeNode*>(left.get());
        string_piece pragmaCode = tok.Str();
        string_piece code = pragmaCode.sub_piece(1, pragmaCode.length() - 1);
        auto toks = Tokenizer::Run(code);
        // parse pragma
        PragmaParser pragma(this, cNode, toks);
        switch (pragma.DoParse()) {
        case PragmaParser::E_UNSUPPORTED_PRAGMA: // #include
            cNode->AppendCode(tok);
        case PragmaParser::E_SUCCESS:
            return left;
        case PragmaParser::E_UNKNOWN_PRAGMA:
            SpawnError("Failed to parse PRAGMA DIRECTIVE", "Unknown pragma");
            return left;
        case PragmaParser::E_UNEXPECTED_TOKEN:
            SpawnError("Failed to parse PRAGMA token", "Unknown token");
            return left;
        }
        return left;
    }

    PragmaParser::PragmaParser(
        Parser* inParser, CodeNode* inCodeNode,
        const std::vector<Token>& Toks)
        : BaseParser(Toks)
        , m_Error(PragmaParser::E_SUCCESS)
        , m_Parent(inParser)
        , m_CodeNode(inCodeNode) {
    }
    PragmaParser::~PragmaParser()
    {}
    PragmaParser::ErrorCode PragmaParser::DoParse()
    {
        if (!m_Tokens.empty()) {
            auto tok = Consume();
            if (tok.Str() == "pragma") {
                m_Error = PragmaParser::E_SUCCESS;
            } else {
                m_Error = PragmaParser::E_UNSUPPORTED_PRAGMA;
                return m_Error;
            }
        }
        if (!AtEnd()) {
            auto tok = Consume();
            FnPPrefix ppfn = s_Pragmas[tok.Str()];
            if (ppfn) {
                auto node = (this->*ppfn)(tok);
                if (node) {
                    m_CodeNode->AppendPragma(std::move(node));
                } else {
                    return m_Error;
                }
            } else {
                m_Error = PragmaParser::E_UNKNOWN_PRAGMA;
                return m_Error;
            }
        }
        return m_Error;
    }
    std::unique_ptr<PragmaNode> PragmaParser::VertexFunction(Token const & tok)
    {
        auto cur = Cur();
        if (!Match(Token::TOK_IDENTIFIER)) {
            m_Error = PragmaParser::E_UNEXPECTED_TOKEN;
            return nullptr;
        }
        auto pNode = std::make_unique<PragmaNode>(PragmaNode::PRAGMA_VERTEX_ENTRY);
        pNode->m_Entry = (String)cur.Str();
        return pNode;
    }
    std::unique_ptr<PragmaNode> PragmaParser::FragmentFunction(Token const & tok)
    {
        auto cur = Cur();
        if (!Match(Token::TOK_IDENTIFIER)) {
            m_Error = PragmaParser::E_UNEXPECTED_TOKEN;
            return nullptr;
        }
        auto pNode = std::make_unique<PragmaNode>(PragmaNode::PRAGMA_FRAGMENT_ENTRY);
        pNode->m_Entry = (String)cur.Str();
        return pNode;
    }
    std::unique_ptr<PragmaNode> PragmaParser::GeometryFunction(Token const & tok)
    {
        auto cur = Cur();
        if (!Match(Token::TOK_IDENTIFIER)) {
            m_Error = PragmaParser::E_UNEXPECTED_TOKEN;
            return nullptr;
        }
        auto pNode = std::make_unique<PragmaNode>(PragmaNode::PRAGMA_GEOMETRY_ENTRY);
        pNode->m_Entry = (String)cur.Str();
        return pNode;
    }
    std::unique_ptr<PragmaNode> PragmaParser::TesselateFunction(Token const & tok)
    {
        auto cur = Cur();
        if (!Match(Token::TOK_IDENTIFIER)) {
            m_Error = PragmaParser::E_UNEXPECTED_TOKEN;
            return nullptr;
        }
        auto pNode = std::make_unique<PragmaNode>(PragmaNode::PRAGMA_TESSELATE_ENTRY);
        pNode->m_Entry = (String)cur.Str();
        return pNode;
    }
    std::unique_ptr<PragmaNode> PragmaParser::ShaderFeature(Token const & tok)
    {
        auto cur = Cur();
        if (!Match(Token::TOK_IDENTIFIER)) {
            m_Error = PragmaParser::E_UNEXPECTED_TOKEN;
            return nullptr;
        }
        auto pNode = std::make_unique<PragmaNode>(PragmaNode::PRAGMA_SHADER_FEATURE);
        pNode->m_ShaderFeature = (String)cur.Str();
        return pNode;
    }
    std::unique_ptr<PragmaNode> PragmaParser::MultiCompile(Token const & tok)
    {
        auto pNode = std::make_unique<PragmaNode>(PragmaNode::PRAGMA_MULTICOMPILE);
        while (!AtEnd()) {
            auto cur = Cur();
            if (!Match(Token::TOK_IDENTIFIER)) {
                m_Error = PragmaParser::E_UNEXPECTED_TOKEN;
                return nullptr;
            }
            pNode->m_MultiCompileMacros.push((String)cur.Str());
        }
        return pNode;
    }
    std::unique_ptr<PragmaNode> PragmaParser::ShaderTarget(Token const & tok)
    {
        auto cur = Cur();
        if (!Match(Token::TOK_FLOAT)) {
            m_Error = PragmaParser::E_UNEXPECTED_TOKEN;
            return nullptr;
        }
        auto pNode = std::make_unique<PragmaNode>(PragmaNode::PRAGMA_SHADER_TARGET);
        pNode->m_ShaderProfile;
        return pNode;
    }
    std::unique_ptr<PragmaNode> PragmaParser::OnlyRenderers(Token const & tok)
    {
        while (!AtEnd()) {
            auto cur = Cur();
            if (!Match(Token::TOK_IDENTIFIER)) {
                m_Error = PragmaParser::E_UNEXPECTED_TOKEN;
                break;
            }
        }
        auto pNode = std::make_unique<PragmaNode>(PragmaNode::PRAGMA_ONLY_RENDERERS);
        pNode->m_Renderers;
        return pNode;
    }
}