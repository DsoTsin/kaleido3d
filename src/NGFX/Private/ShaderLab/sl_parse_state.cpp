#include "sl_parser.h"

namespace shaderlab
{
    //@see https://docs.unity3d.com/Manual/SL-Stencil.html
    //@see https://docs.unity3d.com/Manual/SL-CullAndDepth.html
    static std::unordered_map<String, ngfx::ComparisonFunction> comparisonKeywords = {
    { "Less",       ngfx::ComparisonFunction::Less},
    { "Greater",    ngfx::ComparisonFunction::Greater },
    { "LEqual",     ngfx::ComparisonFunction::LessEqual },
    { "GEqual",     ngfx::ComparisonFunction::GreaterEqual },
    { "Equal",      ngfx::ComparisonFunction::Equal },
    { "NotEqual",   ngfx::ComparisonFunction::NotEqual },
    { "Always",     ngfx::ComparisonFunction::Always },
    { "Off",        ngfx::ComparisonFunction::Never },
    { "Never",      ngfx::ComparisonFunction::Never },
    };

    static std::unordered_map<String, ngfx::CullMode> cullKeywords = {
    { "Back",       ngfx::CullMode::Back },
    { "Front",      ngfx::CullMode::Front },
    { "Off",        ngfx::CullMode::None },
    };

    static std::unordered_map<string_piece, ngfx::BlendOperation> blendOpKeywords = {
    { "Add",    ngfx::BlendOperation::Add},
    { "Sub",    ngfx::BlendOperation::Sub },
    { "RevSub", ngfx::BlendOperation::RevSub },
    { "Min",    ngfx::BlendOperation::Min },
    { "Max",    ngfx::BlendOperation::Max },
    //{ "LogicalClear", ngfx::BlendOperation::Add },
    //{ "LogicalSet", ngfx::BlendOperation::Add },
    };

    static bool IsBlendOperation(string_piece const& str) {
        return blendOpKeywords.find(str) != blendOpKeywords.end();
    }

    static std::unordered_map<string_piece, ngfx::BlendFactor> blendFactorKeywords = {
    { "One",                ngfx::BlendFactor::Zero },
    { "Zero",               ngfx::BlendFactor::One },
    { "SrcColor",           ngfx::BlendFactor::SrcColor },
    { "SrcAlpha",           ngfx::BlendFactor::SrcAlpha },
    { "DstColor",           ngfx::BlendFactor::DestColor },
    { "DstAlpha",           ngfx::BlendFactor::DestAlpha },
    { "OneMinusSrcColor",   ngfx::BlendFactor::OneMinusSrcColor },
    { "OneMinusSrcAlpha",   ngfx::BlendFactor::OneMinusSrcAlpha },
    { "OneMinusDstColor",   ngfx::BlendFactor::OneMinusDestColor },
    { "OneMinusDstAlpha",   ngfx::BlendFactor::OneMinusDestAlpha },
    };

    static bool IsBlendFactor(string_piece const& str) {
        return blendFactorKeywords.find(str) != blendFactorKeywords.end();
    }

    static std::unordered_map<String, bool> switchKeywords = {
    { "On",  true},
    { "Off", false}
    };

    static std::unordered_map<String, ngfx::StencilOperation> stencilKeywords = {
    { "Zero",       ngfx::StencilOperation::Zero },
    { "Keep",       ngfx::StencilOperation::Keep },
    { "Replace",    ngfx::StencilOperation::Replace },
    { "IncrSat",    ngfx::StencilOperation::IncrementClamp },
    { "DecrSat",    ngfx::StencilOperation::DecrementClamp},
    { "Invert",     ngfx::StencilOperation::Invert },
    { "IncrWrap",   ngfx::StencilOperation::IncrementWrap },
    { "DecrWrap",   ngfx::StencilOperation::DecrementWrap },
    };

    std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > Parser::s_StencilExpr = {
    { Token::TOK_COMMENT,           { nullptr, &Parser::Comment } },
    { Token::TOK_STATE_PASS,        { nullptr, &Parser::StencilPass } },
    { Token::TOK_STATE_FAIL,        { nullptr, &Parser::StencilFail } },
    { Token::TOK_STATE_ZFAIL,       { nullptr, &Parser::StencilZFail } },
    { Token::TOK_STATE_COMP,        { nullptr, &Parser::StencilComp } },
    { Token::TOK_STATE_REF,         { nullptr, &Parser::StencilRef } },
    { Token::TOK_STATE_READMASK,    { nullptr, &Parser::StencilReadMask } },
    { Token::TOK_STATE_WRITEMASK,   { nullptr, &Parser::StencilWriteMask } },
    };

    bool IsComparisonFunction(Token const & tok)
    {
        auto tokStr = (std::string)tok.Str();
        return comparisonKeywords.find(tokStr) != comparisonKeywords.end();
    }

    bool IsStencilOperation(Token const & tok)
    {
        auto tokStr = (std::string)tok.Str();
        return stencilKeywords.find(tokStr) != stencilKeywords.end();
    }

    ngfx::DepthStencilState* Parser::ExtractDepthStencil(ParserNode* left)
    {
        ngfx::DepthStencilState* depthStencil = nullptr;
        if (left->AsSubShader()) {
            depthStencil = &(static_cast<SubShaderNode*>(left)->m_State.DS);
        } else if (left->AsPass()) {
            depthStencil = &(static_cast<PassNode*>(left)->m_State.DS);
        } else {
            
        }
        return depthStencil;
    }

    ngfx::BlendState* Parser::ExtractBlend(ParserNode * left)
    {
        ngfx::BlendState* BS = nullptr;
        if (left->AsSubShader()) {
            BS = &(static_cast<SubShaderNode*>(left)->m_State.BS);
        } else if (left->AsPass()) {
            BS = &(static_cast<PassNode*>(left)->m_State.BS);
        } else {

        }
        return BS;
    }

    ngfx::RasterizerState* Parser::ExtractRaster(ParserNode * left)
    {
        ngfx::RasterizerState* RS = nullptr;
        if (left->AsSubShader()) {
            RS = &(static_cast<SubShaderNode*>(left)->m_State.RS);
        } else if (left->AsPass()) {
            RS = &(static_cast<PassNode*>(left)->m_State.RS);
        } else {

        }
        return RS;
    }

    // ZTest Less | Greater | LEqual | GEqual | Equal | NotEqual | Always | Off
    NodePtr Parser::ZTest(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        auto CurTok = Consume();
        std::string curWord = CurTok.Str().to_string();
        // unrecognize
        if (comparisonKeywords.find(curWord) == comparisonKeywords.end()) {
            SpawnError("Unrecognized ZTest value", "Expected is Less | Greater | LEqual | GEqual | Equal | NotEqual | Always | Off");
            return left;
        }
        ngfx::DepthStencilState* depthStencil = ExtractDepthStencil(left.get());
        ngfx::ComparisonFunction cf = comparisonKeywords[curWord];
        if (cf != ngfx::ComparisonFunction::Never) {
            depthStencil->depthTest = true;
            depthStencil->depthFunction = cf;
        } else {
            depthStencil->depthTest = false;
        }
        return left;
    }
    // ZWrite On | Off
    NodePtr Parser::ZWrite(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        auto CurTok = Consume();
        std::string curWord = CurTok.Str().to_string();
        if (switchKeywords.find(curWord) == switchKeywords.end()) {
            SpawnError("Unrecognized ZWrite value", "Expected is On | Off");
            return left;
        }
        ngfx::DepthStencilState* depthStencil = ExtractDepthStencil(left.get());
        if (curWord == "On") {
            depthStencil->depthWriteMask = (ngfx::DepthWriteMask::All);
        } else {
            depthStencil->depthWriteMask = (ngfx::DepthWriteMask::Zero);
        }
        return left;
    }
    // ZClip NUMBER
    NodePtr Parser::ZClip(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        Token CurTok;
        if (!ConsumeNumber(CurTok)) {
            SpawnError("Unrecognized param after ZClip state", "ZClip Only accepted NUMBER");
        }
        return left;
    }
    // Offset Factor(Int), Units(Int)
    NodePtr Parser::Offset(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        Token tokNo;
        if (!ConsumeNumber(tokNo)) {
            SpawnError("Failed to parse factor param in Offset state", "Expected NUMBER here");
            return left;
        }
        double depthSlope = tokNo.AsFloat();
        if (!Match(Token::TOK_COMMA)) {
            SpawnError("Unrecognized token in Offset state", "Expected ',' here");
            return left;
        }
        if (!ConsumeNumber(tokNo)) {
            SpawnError("Failed to parse unit param after Offset state", "Expected NUMBER here");
            return left;
        }
        double depthBias = tokNo.AsFloat();

        ngfx::RasterizerState* RS = ExtractRaster(left.get());
        RS->depthBiasSlope = ((float)depthSlope);
        RS->depthBias = ((int32_t)depthBias);
        
        return left;
    }
    // Stencil Off | BLOCK
    NodePtr Parser::Stencil(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        auto CurTok = (std::string)Cur().Str();
        ngfx::DepthStencilState* depthStencil = ExtractDepthStencil(left.get());
        if (LookAhead(Token::TOK_LEFT_BRACE)) {
            auto Stencil = StencilBlock();
            // 
        } else if (CurTok == "Off") {
            depthStencil->stencilTest = (false);
        } else {
            SpawnError("Parsing stencil state failed", "Expected \' or 'Off' here");
        }
        return left;
    }
    //{
    //   WriteMask 
    //   ReadMask
    //   Ref
    //   Comp
    //   Pass
    //   Fail
    //   ZFail
    // }
    NodePtr Parser::StencilBlock()
    {
        if (!Match(Token::TOK_LEFT_BRACE))
            return nullptr;
        NodePtr stencilNode = std::make_unique<StencilNode>();
        while (!AtEnd()) {
            if (LookAhead(Token::TOK_RIGHT_BRACE)) {
                Consume();
                break;
            }
            auto Next = Consume();
            FnInfix infix = s_StencilExpr[Next.GetType()].second;
            if (infix) {
                stencilNode = (this->*infix)(std::move(stencilNode), Next);
            } else {
                SpawnError("Parsing stencil block failed", "Unexpected token");
                break;
            }

            if (AtEnd()) {
                SpawnError("Unexpected EoF in Stencil block", "'}' expected");
                return nullptr;
            }
        }
        return stencilNode;
    }
    NodePtr Parser::StencilWriteMask(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        auto curTok = Cur();
        if (!Match(Token::TOK_INTEGER)) {
            SpawnError("Failed to parse stencil write mask value", "Expected integer here");
            return left;
        }
        StencilNode* stNode = static_cast<StencilNode*>(left.get());
        stNode->m_WriteMask = curTok.Int();
        return left;
    }
    NodePtr Parser::StencilReadMask(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        auto curTok = Cur();
        if (!Match(Token::TOK_INTEGER)) {
            SpawnError("Failed to parse stencil read mask value", "Expected integer here");
            return left;
        }
        StencilNode* stNode = static_cast<StencilNode*>(left.get());
        stNode->m_ReadMask = curTok.Int();
        return left;
    }
    NodePtr Parser::StencilRef(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        auto curTok = Cur();
        if (!Match(Token::TOK_INTEGER)) {
            SpawnError("Failed to parse stencil ref value", "Expected integer here");
            return left;
        }
        StencilNode* stNode = static_cast<StencilNode*>(left.get());
        stNode->m_Ref = curTok.Int();
        return left;
    }
    NodePtr Parser::StencilComp(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        if (!IsComparisonFunction(Cur())) {
            SpawnError("Failed to parse stencil comparison function value", "Expected integer here");
            return left;
        }
        StencilNode* stNode = static_cast<StencilNode*>(left.get());
        stNode->m_Comp = comparisonKeywords[(std::string)Consume().Str()];
        return left;
    }
    NodePtr Parser::StencilPass(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        if (!IsStencilOperation(Cur())) {
            SpawnError("Parsing pass stencil operation failed");
            return left;
        }
        StencilNode* stNode = static_cast<StencilNode*>(left.get());
        stNode->m_Pass = stencilKeywords[(std::string)Consume().Str()];
        return left;
    }
    NodePtr Parser::StencilFail(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        if (!IsStencilOperation(Cur())) {
            SpawnError("Parsing fail stencil operation failed");
            return left;
        }
        StencilNode* stNode = static_cast<StencilNode*>(left.get());
        stNode->m_Fail = stencilKeywords[(std::string)Consume().Str()];
        return left;
    }
    NodePtr Parser::StencilZFail(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        if (!IsStencilOperation(Cur())) {
            SpawnError("Parsing zfail stencil operation failed");
            return left;
        }
        StencilNode* stNode = static_cast<StencilNode*>(left.get());
        stNode->m_ZFail = stencilKeywords[(std::string)Consume().Str()];
        return left;
    }
    // Blend Off
    //       [Num] One Zero SrcColor SrcAlpha DstColor DstAlpha OneMinusSrcColor OneMinusSrcAlpha OneMinusDstColor OneMinusDstAlpha
    // Blend N SrcFactor DstFactor
    // Blend N SrcFactor DstFactor, SrcFactorA DstFactorA
    // @see https://docs.unity3d.com/Manual/SL-Blend.html
    NodePtr Parser::Blend(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        auto CurTok = Cur();
        ngfx::BlendState* BS = ExtractBlend(left.get());
        if (CurTok.GetType() == Token::TOK_IDENTIFIER) {
            if (!ParseBlendFactor(BS->renderTargets[0])) {
                SpawnError("Parsing blend factor for RenderTarget0 failed");
                return left;
            }
        } else if (CurTok.GetType() == Token::TOK_INTEGER) {
            int target = Consume().Int();
            if (!ParseBlendFactor(BS->renderTargets[target])) {
                SpawnError("Parsing blend factor for RenderTarget failed");
                return left;
            }
        } else {
            SpawnError("Parsing blend factor failed, unexpected token", "");
            return left;
        }
        return left;
    }
    bool Parser::ParseBlendFactor(ngfx::RenderTargetBlendState& src)
    {
        std::string tokStr = (std::string)Cur().Str();
        if (tokStr == "Off") {
            Consume();
            src.blendEnable = false;
        } else if (IsBlendFactor(Cur().Str())) {
            if (!ParseBlendFactor(src.srcColor, src.destColor)) {
                return false;
            }
            if (LookAhead(Token::TOK_COMMA)) { // Alpha
                Consume();
                if (!ParseBlendFactor(src.srcAlpha, src.destAlpha)) {
                    return false;
                }
            }
            src.blendEnable = true;
        } else {
            SpawnError("Unexpected blend factor here", "See syntax on https://docs.unity3d.com/Manual/SL-Blend.html");
            return false;
        }
        return true;
    }
    bool Parser::ParseBlendFactor(ngfx::BlendFactor & src, ngfx::BlendFactor & dst)
    {
        if (!LookAhead(Token::TOK_IDENTIFIER) || !IsBlendFactor(Cur().Str())) {
            SpawnError("Parsing source Blend factor failed", "Unexpected token !");
            return false;
        }
        src = blendFactorKeywords[Consume().Str()];
        if (!LookAhead(Token::TOK_IDENTIFIER) || !IsBlendFactor(Cur().Str()))
        {
            SpawnError("Parsing dest Blend factor failed", "Unexpected token !");
            return false;
        }
        dst = blendFactorKeywords[Consume().Str()];
        return true;
    }
    // BlendOp [N] Op
    // BlendOp [N] OpColor, OpAlpha
    // Op: Add Sub RevSub Min Max LogicalClear LogicalSet LogicalNoop LogicalInvert LogicalAnd LogicalNand LogicalOr
    // @see https://docs.unity3d.com/Manual/SL-Blend.html
    NodePtr Parser::BlendOp(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        ngfx::BlendState* BS = ExtractBlend(left.get());
        auto CurTok = Cur();
        if (Match(Token::TOK_INTEGER)) {
            if (!ParseBlendOperation(BS->renderTargets[CurTok.Int()].colorOp,
                BS->renderTargets[CurTok.Int()].alphaOp)) {
                SpawnError("Failed to blend operation with Dest RenderTarget.");
                return left;
            }
        } else if (CurTok.GetType() == Token::TOK_IDENTIFIER) {
            if (!ParseBlendOperation(BS->renderTargets[0].colorOp,
                BS->renderTargets[0].alphaOp)) {
                SpawnError("Failed to blend operation with RenderTarget0");
                return left;
            }
        } else {
            SpawnError("Failed to blend operation with RenderTarget0", "Unexpected token");
        }

        return left;
    }
    bool Parser::ParseBlendOperation(ngfx::BlendOperation & color, ngfx::BlendOperation & alpha)
    {
        if (!IsBlendOperation(Cur().Str())) {
            SpawnError("Failed to parse color blend operation");
            return false;
        }

        color = blendOpKeywords[Consume().Str()];

        if (Match(Token::TOK_COMMA)) {
            Token alphaTok = Consume();
            if (!IsBlendOperation(alphaTok.Str())) {
                SpawnError("Failed to parse alpha blend operation");
                return false;
            }
            
            alpha = blendOpKeywords[alphaTok.Str()];
        }

        return true;
    }
    // AlphaToMask On
    NodePtr Parser::AlphaToMask(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        return left;
    }
    // AlphaTest 
    NodePtr Parser::AlphaTest(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        return left;
    }


    static bool ParseColorMask(string_piece const& cmStr, ngfx::RenderTargetBlendState& rbs)
    {
        uint32_t mask = 0;
        for (int i = 0; i < cmStr.length(); i++) {
            if (cmStr[i] == 'R') {
                mask |= 0xffff000000000000;
            }
            else if (cmStr[i] == 'G') {
                mask |= 0x0000ffff00000000;
            }
            else if (cmStr[i] == 'B') {
                mask |= 0x00000000ffff0000;
            }
            else if (cmStr[i] == 'A') {
                mask |= 0x000000000000ffff;
            }
        }
        rbs.colorWriteMask = mask;
        return true;
    }

    // ColorMask 0 | RGBA
    NodePtr Parser::ColorMask(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        ngfx::BlendState* BS = ExtractBlend(left.get());
        if (LookAhead(Token::TOK_INTEGER)) {
            Token mask = Consume();
            BS->renderTargets[0].colorWriteMask = mask.Int();
        } else if (LookAhead(Token::TOK_IDENTIFIER)) {
            auto cmTok = Consume().Str();
            if(!ParseColorMask(cmTok, BS->renderTargets[0])) {
                SpawnError("Failed to parse ColorMask value", "Any of 'RGBA' here");
                return left;
            }
        } else {
            SpawnError("Failed to parse ColorMask", "Expected Integer or Any of 'RGBA' here");
            return left;
        }
        return left;
    }
    // Fog {}
    NodePtr Parser::Fog(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        if (!Match(Token::TOK_LEFT_BRACE)) {
            SpawnError("Failed to parse Fog state", "Expected { here");
            return left;
        }
        while (!AtEnd()) {
            if (LookAhead(Token::TOK_RIGHT_BRACE)) {
                Consume();
                break;
            }
            Consume();
        }
        return left;
    }
    // Cull Back | Front | Off
    NodePtr Parser::Cull(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        auto CurTok = Consume();
        std::string curWord = (std::string)CurTok.Str();
        if (cullKeywords.find(curWord) == cullKeywords.end()) {
            SpawnError("Unrecognized Cull state value", "Expected is Back | Front | Off");
            return left;
        }
        ngfx::RasterizerState* RS = ExtractRaster(left.get());
        RS->cullMode = cullKeywords[curWord];
        return left;
    }
}