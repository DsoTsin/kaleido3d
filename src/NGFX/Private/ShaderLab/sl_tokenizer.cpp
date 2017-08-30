#include "sl_tokenizer.h"
#include <unordered_map>
#include <sstream>
namespace shaderlab
{
    using namespace std;
    static bool IsWhiteSpace(char c)
    {
        return c == 0x0A || c == 0x0D || c == 0x20 || c == 0x09;
    }
    static bool IsNewLine(char c)
    {
        return c == '\n';
    }
    static bool IsDigital(char c)
    {
        return c >= '0' && c <= '9';
    }
    static bool IsAlpha(char c)
    {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }
    static bool IsIdentifierFirstChar(char c)
    {
        return IsAlpha(c) || c == '_';
    }
    static bool IsIdentifierContinuingChar(char c)
    {
        return IsIdentifierFirstChar(c) || IsDigital(c);
    }
    bool IsHLSLBlockBegin(const string_piece& str) {
        auto _Str = str.to_string();
        return _Str == "HLSLINCLUDE" || _Str == "HLSLPROGRAM" || _Str == "ENDHLSL";
    }
    Token::Type GetSpecialType(const string_piece& str) {
        auto _Str = str.to_string();
        if (_Str == "HLSLINCLUDE")
            return Token::TOK_HLSLINCLUDE;
        else if(_Str == "HLSLPROGRAM")
            return Token::TOK_HLSLPROG;
        else if (_Str == "ENDHLSL")
            return Token::TOK_HLSLEND;
        return Token::TOK_IDENTIFIER;
    }

    static std::unordered_map<std::string, Token::Type> s_KeywordMap = {
        // Meta Node
        { "Shader",         Token::TOK_SHADER },
        { "SubShader",      Token::TOK_SUBSHADER },
        { "Pass",           Token::TOK_PASS },
        { "Name",           Token::TOK_PASS_NAME },
        { "Properties",     Token::TOK_PROPERTIES },
        { "Tags",           Token::TOK_TAGS },
        { "Fallback",       Token::TOK_FALLBACK },      // FallBack
        { "HLSLINCLUDE",    Token::TOK_HLSLINCLUDE },
        { "HLSLPROGRAM",    Token::TOK_HLSLPROG },
        { "ENDHLSL",        Token::TOK_HLSLEND },
        // Properties
        { "Range",          Token::TOK_PROP_RANGE },
        { "Color",          Token::TOK_PROP_COLOR },
        { "Vector",         Token::TOK_PROP_VECTOR },
        { "1D",             Token::TOK_PROP_SAMPLER_1D },
        { "2D",             Token::TOK_PROP_SAMPLER_2D },
        { "3D",             Token::TOK_PROP_SAMPLER_3D },
        { "Cube",           Token::TOK_PROP_SAMPLER_CUBE },
        { "Float",          Token::TOK_PROP_FLOAT },
        { "Int",            Token::TOK_PROP_INTEGER },
        { "UInt",           Token::TOK_PROP_INTEGER },
        // State Keywords
        { "Blend",          Token::TOK_STATE_BLEND },
        { "BlendOp",        Token::TOK_STATE_BLENDOP },
        { "AlphaToMask",    Token::TOK_STATE_ALPHATOMASK },
        { "ColorMask",      Token::TOK_STATE_COLORMASK },
        { "Fog",            Token::TOK_STATE_FOG },
        { "AlphaTest",      Token::TOK_STATE_ALPHATEST },
        { "Cull",           Token::TOK_STATE_CULL },
        { "ZTest",          Token::TOK_STATE_ZTEST },
        { "ZClip",          Token::TOK_STATE_ZTEST },
        { "ZWrite",         Token::TOK_STATE_ZWRITE },
        { "Offset",         Token::TOK_STATE_OFFSET },
        { "Stencil",        Token::TOK_STATE_STENCIL },
        // State Comparator
        { "Less",           Token::TOK_STATE_LESS },
        { "Greater",        Token::TOK_STATE_GREATER },
        { "LEqual",         Token::TOK_STATE_LESSEQUAL },
        { "GEqual",         Token::TOK_STATE_GREATEEQUAL },
        { "Equal",          Token::TOK_STATE_EQUAL },
        { "NotEqual",       Token::TOK_STATE_NOTEQUAL },
        { "Always",         Token::TOK_STATE_ALWAYS },
        // Stencil
        { "Ref",            Token::TOK_STATE_REF },
        { "ReadMask",       Token::TOK_STATE_READMASK },
        { "WriteMask",      Token::TOK_STATE_WRITEMASK },
        { "Comp",           Token::TOK_STATE_COMP },
        { "Fail",           Token::TOK_STATE_FAIL },
        { "ZFail",          Token::TOK_STATE_ZFAIL },
        // Stencil Op
        { "Keep",           Token::TOK_STATE_KEEP },
        { "Zero",           Token::TOK_STATE_ZERO },
        { "Replace",        Token::TOK_STATE_REPLACE },
        { "IncrSat",        Token::TOK_STATE_INCRSAT },
        { "DecrSat",        Token::TOK_STATE_DECRSAT },
        { "Invert",         Token::TOK_STATE_INVERT },
        { "IncrWrap",       Token::TOK_STATE_INCRWRAP },
        { "DecrWrap",       Token::TOK_STATE_DECRWRAP },
    };

    static std::unordered_map<std::string, Token::Type> s_KeywordAcceptInShader = {
    { "Shader",         Token::TOK_SHADER },
    { "SubShader",      Token::TOK_SUBSHADER },
    { "Properties",     Token::TOK_PROPERTIES },
    { "Fallback",       Token::TOK_FALLBACK },      // FallBack
    { "HLSLINCLUDE",    Token::TOK_HLSLINCLUDE },
    { "ENDHLSL",        Token::TOK_HLSLEND },
    { "CustomEditor",   Token::TOK_CUSTOM_EDITOR },
    };

    static std::unordered_map<std::string, Token::Type> s_KeywordAcceptInProperties = {
    { "Range",          Token::TOK_PROP_RANGE },
    { "Color",          Token::TOK_PROP_COLOR },
    { "Vector",         Token::TOK_PROP_VECTOR },
    { "1D",             Token::TOK_PROP_SAMPLER_1D },
    { "2D",             Token::TOK_PROP_SAMPLER_2D },
    { "3D",             Token::TOK_PROP_SAMPLER_3D },
    { "Cube",           Token::TOK_PROP_SAMPLER_CUBE },
    { "Float",          Token::TOK_PROP_FLOAT },
    { "Int",            Token::TOK_PROP_INTEGER },
    { "UInt",           Token::TOK_PROP_INTEGER },
    };

    static std::unordered_map<std::string, Token::Type> s_KeywordAcceptInSubShader = {
    { "HLSLPROGRAM",    Token::TOK_HLSLPROG },
    { "ENDHLSL",        Token::TOK_HLSLEND },
    { "Pass",           Token::TOK_PASS },
    { "Tags",           Token::TOK_TAGS },
    { "LOD",            Token::TOK_LOD },

    { "Blend",          Token::TOK_STATE_BLEND },
    { "BlendOp",        Token::TOK_STATE_BLENDOP },
    { "AlphaToMask",    Token::TOK_STATE_ALPHATOMASK },
    { "ColorMask",      Token::TOK_STATE_COLORMASK },
    { "Fog",            Token::TOK_STATE_FOG },
    { "AlphaTest",      Token::TOK_STATE_ALPHATEST },
    { "Cull",           Token::TOK_STATE_CULL },
    { "ZTest",          Token::TOK_STATE_ZTEST },
    { "ZClip",          Token::TOK_STATE_ZTEST },
    { "ZWrite",         Token::TOK_STATE_ZWRITE },
    { "Offset",         Token::TOK_STATE_OFFSET },
    { "Stencil",        Token::TOK_STATE_STENCIL },
    };

    static std::unordered_map<std::string, Token::Type> s_KeywordAcceptInPass = {
    { "HLSLPROGRAM",    Token::TOK_HLSLPROG },
    { "ENDHLSL",        Token::TOK_HLSLEND },
    { "Tags",           Token::TOK_TAGS },
    { "LOD",            Token::TOK_LOD },
    { "Name",           Token::TOK_PASS_NAME },

    { "Blend",          Token::TOK_STATE_BLEND },
    { "BlendOp",        Token::TOK_STATE_BLENDOP },
    { "AlphaToMask",    Token::TOK_STATE_ALPHATOMASK },
    { "ColorMask",      Token::TOK_STATE_COLORMASK },
    { "Fog",            Token::TOK_STATE_FOG },
    { "AlphaTest",      Token::TOK_STATE_ALPHATEST },
    { "Cull",           Token::TOK_STATE_CULL },
    { "ZTest",          Token::TOK_STATE_ZTEST },
    { "ZClip",          Token::TOK_STATE_ZTEST },
    { "ZWrite",         Token::TOK_STATE_ZWRITE },
    { "Offset",         Token::TOK_STATE_OFFSET },
    { "Stencil",        Token::TOK_STATE_STENCIL },

    };

    static std::unordered_map<std::string, Token::Type> s_KeywordAcceptInState = {
    { "Blend",          Token::TOK_STATE_BLEND },
    { "BlendOp",        Token::TOK_STATE_BLENDOP },
    { "AlphaToMask",    Token::TOK_STATE_ALPHATOMASK },
    { "ColorMask",      Token::TOK_STATE_COLORMASK },
    { "Fog",            Token::TOK_STATE_FOG },
    { "AlphaTest",      Token::TOK_STATE_ALPHATEST },
    { "Cull",           Token::TOK_STATE_CULL },
    { "ZTest",          Token::TOK_STATE_ZTEST },
    { "ZClip",          Token::TOK_STATE_ZTEST },
    { "ZWrite",         Token::TOK_STATE_ZWRITE },
    { "Offset",         Token::TOK_STATE_OFFSET },
    { "Stencil",        Token::TOK_STATE_STENCIL },
    };

    static std::unordered_map<std::string, Token::Type> s_KeywordAcceptInStencil = {
    { "Ref",            Token::TOK_STATE_REF },
    { "ReadMask",       Token::TOK_STATE_READMASK },
    { "WriteMask",      Token::TOK_STATE_WRITEMASK },
    { "Comp",           Token::TOK_STATE_COMP },
    { "Fail",           Token::TOK_STATE_FAIL },
    { "ZFail",          Token::TOK_STATE_ZFAIL },
    { "Pass",           Token::TOK_STATE_PASS }
    };
    std::string Unquote(std::string const& inStr)
    {
        return inStr.substr(1, inStr.length() - 2);
    }
    string_piece Unquote(string_piece const& inStr)
    {
        return string_piece(inStr.begin() + 1, inStr.end() - 1);
    }
    bool IsKeyword(string_piece const& Str)
    {
        return s_KeywordMap.find(Str.to_string()) != s_KeywordMap.end();
    }
    Token::Token() : m_Type(Token::TOK_INVALID), m_Float(0.0), m_Integer(0)
    {}
    Token::Token(Location const& loc, Type const& t, const string_piece& str, double fVal, int iVal)
    : m_Str(str), m_Type(t), m_Location(loc), m_Float(fVal), m_Integer(iVal) {}
    Token& Token::operator=(Token const & Rhs)
    {
        m_Location = Rhs.m_Location;
        m_Type = Rhs.m_Type;
        m_Integer = Rhs.m_Integer;
        m_Str = Rhs.m_Str;
        m_Boolean = Rhs.m_Boolean;
        m_Float = Rhs.m_Float;
        return *this;
    }
    double Token::AsFloat() const
    {
        if (m_Type == TOK_INTEGER) {
            return 1.0 * m_Integer;
        } else if (m_Type == TOK_FLOAT) {
            return m_Float;
        }
        return 0.0;
    }
    std::vector<Token> Tokenizer::Run(const string_piece& str, const string_piece& filePath, Error* inErr)
    {
        Tokenizer tkn(str, filePath, inErr);
        return tkn.Run();
    }
    Tokenizer::~Tokenizer()
    {
    }
    Tokenizer::Tokenizer(const string_piece& Source, const string_piece& Path, Error* inErr)
        : m_File(Path), m_Source(Source)
        , m_Line(1), m_Column(0), m_Offset(0)
        , m_Err(inErr), m_IsInHLSL(false)
        , m_LastBlockToken(Token::TOK_INVALID)
    {
    }
    std::vector<Token> Tokenizer::Run()
    {
        while (!Done()) {
            StripWhiteSpaces();
            if (Done())
                break;
            Location Loc = GetLocation();
            bool isMlComment = false;
            bool isInHLSL = false;
            Token::Type t = ClassifyCurrent(isMlComment);
            if (t == Token::TOK_INVALID)
            {
                SpawnError("Unexpected Token here", "");
                break;
            }
            int tok_begin = m_Offset;
            int iVal = 0;
            double fVal = 0.0f;
            AdvanceToEndOfToken(Loc, t, iVal, fVal, isMlComment);
            if (HasErr())
                break;
            size_t tok_end = m_Offset;
            string_piece tok_val(m_Source.begin() + tok_begin, m_Source.begin() + tok_end);
            auto tok_str = tok_val.to_string();

            switch (t)
            {
            case Token::TOK_IDENTIFIER:
                MatchKeywords(t, tok_str);
                break;
            case Token::TOK_INTEGER:
            case Token::TOK_FLOAT:
                if (m_Tokens.size() >= 2) {
                    auto& TokLast = m_Tokens.back();
                    if (TokLast.GetType() == Token::TOK_MINUS) {
                        if (m_Tokens[m_Tokens.size() - 2].GetType() != Token::TOK_IDENTIFIER) {
                            TokLast.SetType(t);
                            if (t == Token::TOK_FLOAT) {
                                TokLast.SetFloat(fVal * -1.0f);
                            } else {
                                TokLast.SetInt(iVal * -1);
                            }
                            continue; // skip push to toklist
                        }
                    }
                }
                break;
            case Token::TOK_SOURCE:
                if (tok_val.start_with("ENDHLSL")) {
                    if (tok_val.length() == 7 ||
                        (tok_val.length() > 7 && !IsIdentifierContinuingChar(tok_val[7])))
                    {
                        t = Token::TOK_HLSLEND;
                        m_BlockStack.pop(); // pop hlsl source
                    }
                } else if (tok_val.start_with("#pragma ")) {
                    t = Token::TOK_PRAGMA;
                }
                break;
            }
            Token tok(Loc, t, tok_val, fVal, iVal);
            m_Tokens.push_back(tok);
        }
        if (HasErr()) {
            m_Tokens.clear();
        }
        return m_Tokens;
    }
    Token::Type Tokenizer::ClassifyCurrent(bool& IsMultiLineComment)
    {
        char Cur = CurChar();
        // very special case 
        if (!m_BlockStack.empty() &&
            (m_BlockStack.top() == Token::TOK_HLSLINCLUDE
                || m_BlockStack.top() == Token::TOK_HLSLPROG)) {
            // accept a line of code
            return Token::TOK_SOURCE;
        }
        // parse comment
        if (CanAdvance() && Cur == '/') {
            if (NextChar() == '/') {
                IsMultiLineComment = false;
                return Token::TOK_COMMENT;
            } else if (NextChar() == '*') {
                IsMultiLineComment = true;
                return Token::TOK_COMMENT;
            }
        }
        if (IsIdentifierFirstChar(Cur))
            return Token::TOK_IDENTIFIER;
        if (Cur == '#')
            return Token::TOK_PRAGMA;
        if (IsDigital(Cur))
            return Token::TOK_INTEGER;
        switch (Cur) {
        case '"':
            return Token::TOK_LITERAL_STRING;
        case '[':
            return Token::TOK_LEFT_BRACKET;
        case ']':
            return Token::TOK_RIGHT_BRACKET;
        case '(':
            return Token::TOK_LEFT_PAREN;
        case ')':
            return Token::TOK_RIGHT_PAREN;
        case '{':
            return Token::TOK_LEFT_BRACE;
        case '}':
            return Token::TOK_RIGHT_BRACE;
        case '.':
            return Token::TOK_DOT; // May need parse float
        case ',':
            return Token::TOK_COMMA;
        case '-':
            return Token::TOK_MINUS;
        case '=':
            return Token::TOK_ASSIGN;
        }
        return Token::TOK_INVALID;
    }
    bool Tokenizer::CanAdvance()
    {
        return m_Offset + 1 < m_Source.size();
    }
    bool Tokenizer::CanRecede()
    {
        return m_Offset >= 1;
    }
    bool Tokenizer::AtEnd()
    {
        return m_Offset == m_Source.size();
    }
    bool Tokenizer::Done()
    {
        return AtEnd() || HasErr();
    }
    bool Tokenizer::HasErr()
    {
        return m_Err && m_Err->HasError();
    }
    bool Tokenizer::IsCurrentStringTerminator(char quoteChar)
    {
        if (CurChar() != quoteChar)
            return false;
        int num_backslashes = 0;
        for (int i = static_cast<int>(m_Offset) - 1; i >= 0 && m_Source[i] == '\\'; i--)
            num_backslashes++;
        return (num_backslashes % 2) == 0;
    }
    char Tokenizer::CurChar()
    {
        return m_Source[m_Offset];
    }
    char Tokenizer::NextChar()
    {
        return m_Source[m_Offset + 1];
    }
    Location Tokenizer::GetLocation()
    {
        return Location{ m_File, m_Line, m_Column };
    }
    bool Tokenizer::GetLastToken(Token & Tok)
    {
        if(m_Tokens.empty())
            return false;
        Tok = m_Tokens.back();
        return true;
    }
    void Tokenizer::SpawnError(const std::string & error, const std::string & help)
    {
        if (!m_Err) {
            m_Err = new Error(GetLocation(), error, help);
        } else {
            *m_Err = Error(GetLocation(), error, help);
        }
    }
    void Tokenizer::Advance()
    {
        if (IsNewLine(CurChar())) {
            m_Line++;
            m_Column = 1;
        } else {
            m_Column++;
        }
        m_Offset++;
    }
    void Tokenizer::AdvanceToEndOfToken(Location const& Loc, Token::Type& Type, int& iValue, double& fValue, bool isMultiLineComment)
    {
        char Cur = CurChar();
        switch (Type) {
        case Token::TOK_COMMENT:
        {
            if (isMultiLineComment) {
                Advance(); // skip /
                Advance(); // skip *
                for (;;) {
                    if (AtEnd()) {
                        SpawnError("Parsing multi-line comment failed !", "Expected '*/' here.");
                        break;
                    }
                    if (CanAdvance() && CurChar() == '*'
                        && m_Source[m_Offset + 1] == '/') {
                        Advance();
                        Advance();
                        break;
                    }
                    Advance();
                }
            } else {
                do { Advance(); } while (!AtEnd() && !IsNewLine(CurChar()));
            }
            break;
        }
        case Token::TOK_LITERAL_STRING:
        {
            do {
                if (AtEnd()) {
                    // err, expected(
                    SpawnError("End of file.", "Expected \" here.");
                    break;
                } else if (IsNewLine(CurChar())) {
                    // err, no new line allowed
                    SpawnError("New line isn't allowed inside literal string.", "Expected \" here.");
                    break;
                }
                Advance();
            } while (!IsCurrentStringTerminator(Cur));
            Advance();
            break;
        }
        case Token::TOK_DOT:
        {
            if (CanAdvance()) // may be float
            {
                Advance();
                if (IsDigital(CurChar())) { // parse float .1231244f
                    double fractPart = 0.0;
                    double factor = 0.1;
                    while (IsDigital(CurChar()) && !AtEnd()) {
                        fractPart += (CurChar() - '0') * factor;
                        factor *= 0.1;
                        Advance();
                    }
                    if (CurChar() == 'f' || CurChar() == 'F') {
                        Advance();
                    } 
                    if (IsIdentifierContinuingChar(CurChar())) {
                        Type = Token::TOK_INVALID;
                    } else {
                        fValue = fractPart;
                        Type = Token::TOK_FLOAT;
                    }
                } else { // just a normal operator
                    break;
                }
            }
            break;
        }
        case Token::TOK_INTEGER:
        {
            if (CanAdvance()) {
                Advance();
                char Cont = CurChar();
                if ((Cur >= '1' && Cur <= '3') // 1D,2D,3D
                    && (Cont == 'D' || Cont == 'd')) {
                    if (CanAdvance()) {
                        Advance();
                        if (!IsIdentifierContinuingChar(NextChar())) {
                            Type = Token::TOK_IDENTIFIER;
                            break;
                        } else {
                            Type = Token::TOK_INVALID;
                            SpawnError("Parsing number failed, unexpeced token here", "Expected '1D', '2D', '3D' here.");
                            break;
                        }
                    } else { // 
                        Advance();
                        Type = Token::TOK_IDENTIFIER;
                        break;
                    }
                } else { // Parse Number(Int/Float) 32324.45354f 2323E3 4.55E-2
                    bool IsFloat = false;
                    int iNum = 0;
                    int first = m_Offset - 1;
                    int fractPos = first;
                    double fractPart = 0.0;
                    double factor = 0.1;
                    do {
                        if (CurChar() == 'F' || CurChar() == 'f') {
                            Advance();
                            // End of Float
                            fValue = fractPart + 1.0 * iNum;
                            break;
                        }
                        if (CurChar() == '.') {
                            IsFloat = true;
                            int f = 1;
                            for (int i = m_Offset - 1; i >= first; i--) {
                                iNum += (m_Source[i] - '0') * f;
                                f *= 10;
                            }
                            fractPos = m_Offset;
                            Advance();
                        } else if (CurChar() == '-') {
                            Advance();
                        } else if (CurChar() == 'E' || CurChar() == 'e') {
                            IsFloat = true;
                            Advance();
                        } else if (IsDigital(CurChar())) {
                            if (fractPos != first) {
                                fractPart += (CurChar() - '0') * factor;
                                factor *= 0.1;
                            }
                            Advance();
                        } else {
                            if (IsIdentifierContinuingChar(CurChar())) {
                                SpawnError("Parsing number failed, unexpeced token here", "Expected not '_' or [a-zA-Z] .");
                            }
                            break;
                        }
                    } while (!AtEnd());
                    if (IsFloat) {
                        Type = Token::TOK_FLOAT;
                        fValue = fractPart + 1.0 * iNum;
                    } else { // int
                        int f = 1;
                        for (int i = m_Offset - 1; i >= first; i--) {
                            iNum += (m_Source[i] - '0') * f;
                            f *= 10;
                        }
                        iValue = iNum;
                    }
                    break;
                }
            } else {

            }
            break;
        }
        case Token::TOK_IDENTIFIER:
        {
            while (!AtEnd() && IsIdentifierContinuingChar(CurChar()))
                Advance();
            break;
        }
        case Token::TOK_PRAGMA:
        {
            Advance();
            while (!AtEnd() && !IsNewLine(CurChar()))
                Advance();
            Advance();
            break;
        }
        case Token::TOK_SOURCE:
        {
            do { Advance(); } while (!AtEnd() && !IsNewLine(CurChar()));
            break;
        }
        case Token::TOK_HLSLINCLUDE:
        {
            break;
        }
        case Token::TOK_HLSLPROG:
        {
            break;
        }
        case Token::TOK_LEFT_BRACE: {
            Advance();
            switch (m_LastBlockToken) {
            case Token::TOK_SHADER:
            case Token::TOK_PROPERTIES:
            case Token::TOK_SUBSHADER:
            case Token::TOK_PASS:
            case Token::TOK_STATE_STENCIL:
                m_BlockStack.push(m_LastBlockToken);
                break;
            }
            break;
        }
        case Token::TOK_HLSLEND:
            break;
        case Token::TOK_RIGHT_BRACE:
            if(m_LastBlockToken != Token::TOK_INVALID && !m_BlockStack.empty()) {
                switch (m_LastBlockToken) {
                case Token::TOK_SHADER:
                case Token::TOK_PROPERTIES:
                case Token::TOK_SUBSHADER:
                case Token::TOK_PASS:
                case Token::TOK_STATE_STENCIL:
                    m_BlockStack.pop();
                    break;
                }
            }
            Advance();
            break;
        case Token::TOK_ASSIGN:
        case Token::TOK_LEFT_BRACKET:
        case Token::TOK_RIGHT_BRACKET:
        case Token::TOK_LEFT_PAREN:
        case Token::TOK_RIGHT_PAREN:
        case Token::TOK_COMMA:
        case Token::TOK_MINUS:
            Advance();
            break;
        default:
            break;
        }
    }
    void Tokenizer::StripWhiteSpaces()
    {
        while (!AtEnd() && 
            IsWhiteSpace(CurChar())) {
            Advance();
        }
    }
    void Tokenizer::MatchKeywords(Token::Type & Type, std::string const & word)
    {
        if (m_BlockStack.empty()) {
            if (word == "Shader") {
                m_LastBlockToken = Token::TOK_SHADER;
                Type = Token::TOK_SHADER;
            }
        }
        else {
            switch (m_BlockStack.top()) {
            case Token::TOK_SHADER:
                if (s_KeywordAcceptInShader.find(word) != s_KeywordAcceptInShader.end()) {
                    Type = s_KeywordAcceptInShader[word];
                    switch (Type) {
                    case Token::TOK_HLSLINCLUDE:
                        m_BlockStack.push(Type);
                        break;
                    case Token::TOK_SUBSHADER:
                    case Token::TOK_PROPERTIES:
                        m_LastBlockToken = Type;
                        break;
                    }
                }
                break;
            case Token::TOK_PROPERTIES:
                if (s_KeywordAcceptInProperties.find(word) != s_KeywordAcceptInProperties.end()) {
                    Type = s_KeywordAcceptInProperties[word];
                }
                break;
            case Token::TOK_SUBSHADER:
                if (s_KeywordAcceptInSubShader.find(word) != s_KeywordAcceptInSubShader.end()) {
                    Type = s_KeywordAcceptInSubShader[word];
                    switch (Type) {
                    case Token::TOK_HLSLPROG:
                        m_BlockStack.push(Type);
                        break;
                    case Token::TOK_PASS:
                    case Token::TOK_STATE_STENCIL:
                        m_LastBlockToken = Type;
                        break;
                    }
                }
                break;
            case Token::TOK_PASS:
                if (s_KeywordAcceptInPass.find(word) != s_KeywordAcceptInPass.end()) {
                    Type = s_KeywordAcceptInPass[word];
                    switch (Type) {
                    case Token::TOK_HLSLPROG:
                        m_BlockStack.push(Type);
                        break;
                    case Token::TOK_STATE_STENCIL:
                        m_LastBlockToken = Type;
                        break;
                    }
                }
                break;
            case Token::TOK_STATE_STENCIL:
                if (s_KeywordAcceptInStencil.find(word) != s_KeywordAcceptInStencil.end()) {
                    Type = s_KeywordAcceptInStencil[word];
                }
                break;
            }
        }
    }
    std::string Error::PrintError() const
    {
        std::ostringstream oss;
        oss << (std::string)m_Location.Path << "(" << m_Location.Line << "): error : " << m_ErrMsg;
        if(!m_HelpInfo.empty())
            oss << ", suggestion : " << m_HelpInfo;
        return oss.str();
    }
}
