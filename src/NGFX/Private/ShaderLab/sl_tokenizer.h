#pragma once
#include "string_piece.hpp"
#include <vector>
#include <stack>
#include <unordered_map>
namespace shaderlab
{
    using string_piece = std::ext::string_piece;
    struct Location
    {
        string_piece    Path;
        int             Line    = 0;
        int             Column  = 0;

        Location& operator=(Location const& Rhs)
        {
            Path = Rhs.Path;
            Line = Rhs.Line;
            Column = Rhs.Column;
            return *this;
        }
    };
    class Error
    {
    public:
        Error() : m_HasErr(false) {}
        Error(Location const& Loc, std::string const& Err, std::string const& Help = "")
            : m_Location(Loc), m_ErrMsg(Err), m_HelpInfo(Help), m_HasErr(true) {}

        bool HasError() const { return m_HasErr; }
        void SetError(std::string const& Err)  { 
            m_ErrMsg = Err; m_HasErr = true;
        }
        std::string PrintError() const;
    private:
        Location    m_Location;
        std::string m_ErrMsg;
        std::string m_HelpInfo;
        bool        m_HasErr;
    };
    class Token
    {
    public:
        enum Type
        {
            TOK_INVALID,
            TOK_COMMENT,        // 
            TOK_LITERAL_STRING, // "STRING"
            TOK_DOT,            // .
            TOK_COMMA,          // ,
            TOK_MINUS,          // -, Negative Number
            TOK_INTEGER,        // 100
            TOK_ASSIGN,         // =
            TOK_IDENTIFIER,     // [_\w]+, Special "2D"
            TOK_LEFT_BRACE,     // {
            TOK_RIGHT_BRACE,    // }
            TOK_LEFT_BRACKET,   // [
            TOK_RIGHT_BRACKET,  // ]
            TOK_LEFT_PAREN,     // (
            TOK_RIGHT_PAREN,    // )
            TOK_PRAGMA,         // #pragma

            TOK_UNCLASSIFIED,

            TOK_PROPERTIES,     // Properties
            TOK_FLOAT,          // 0.0f
            TOK_ON,             // On
            TOK_OFF,            // Off
            TOK_SHADER,         // Shader
            TOK_SUBSHADER,      // SubShader
            TOK_PASS,           // Pass
            TOK_PASS_NAME,      // Name
            TOK_LOD,
            TOK_TAGS,
            TOK_HLSLINCLUDE,    // HLSLINCLUDE
            TOK_HLSLPROG,       // HLSLPROGRAM
            TOK_HLSLEND,        // ENDHLSL
            TOK_SOURCE,         // Shader Code
            TOK_FALLBACK,       // FallBack
            TOK_CUSTOM_EDITOR,  // CustomEditor

            TOK_PROP_SAMPLER_1D,     // _tex("Cube", 1D) = "white" {}
            TOK_PROP_SAMPLER_2D,     // _tex("Cube", 2D) = "white" {}
            TOK_PROP_SAMPLER_3D,     // _tex("Cube", 3D) = "white" {}
            TOK_PROP_SAMPLER_CUBE,   // _tex("Cube", CUBE) = "white" {}
            TOK_PROP_RANGE,          // _Value("Param", Range(1.0, 5.0)) = 0.5
            TOK_PROP_FLOAT,          // _Value("Float", Float) = 0.25
            TOK_PROP_INTEGER,        // _Value("Int", Int|UInt) = 5
            TOK_PROP_COLOR,          // _Color("Color", Color) = (0.1, 1.0, 2.0, 0.3)
            TOK_PROP_VECTOR,          // _Color("Color", Vector) = (0.1, 1.0, 2.0, 0.3)

            TOK_STATE_BLEND,        // Blend
            TOK_STATE_BLENDOP,      // BlendOp
            TOK_STATE_ALPHATOMASK,  // AlphaToMask
            TOK_STATE_COLORMASK,    // ColorMask
            TOK_STATE_FOG,          // Fog
            TOK_STATE_ALPHATEST,    // AlphaTest
            TOK_STATE_CULL,         // Cull
            TOK_STATE_ZTEST,        // ZTest
            TOK_STATE_ZCLIP,        // ZClip
            TOK_STATE_ZWRITE,       // ZWrite
            TOK_STATE_OFFSET,       // Offset

            TOK_STATE_STENCIL,      // Stencil
            TOK_STATE_REF,          // Ref
            TOK_STATE_READMASK,     // ReadMask
            TOK_STATE_WRITEMASK,    // WriteMask
            TOK_STATE_COMP,         // Comp
            TOK_STATE_PASS,         // Pass, conflicted
            TOK_STATE_FAIL,         // Fail
            TOK_STATE_ZFAIL,        // ZFail
            
            TOK_STATE_LESS,         // Less
            TOK_STATE_GREATER,      
            TOK_STATE_LESSEQUAL,    
            TOK_STATE_GREATEEQUAL,  
            TOK_STATE_EQUAL,        
            TOK_STATE_NOTEQUAL,     
            TOK_STATE_ALWAYS,       

            TOK_STATE_KEEP,         // Keep
            TOK_STATE_ZERO,         // Zero
            TOK_STATE_REPLACE,      // Replace
            TOK_STATE_INCRSAT,      // IncrSat
            TOK_STATE_DECRSAT,      // DecrSat
            TOK_STATE_INVERT,       // Invert
            TOK_STATE_INCRWRAP,     // IncrWrap
            TOK_STATE_DECRWRAP,     // DecrWrap
        };

        Token();
        Token(Location const& loc, Type const& t, const string_piece& str, double fVal, int iVal);

        Token&              operator=(Token const& Rhs);

        Token::Type         GetType() const { return m_Type; }
        void                SetType(Type const& t) { m_Type = t; }
        const Location&     GetLocation() const { return m_Location; }
        const string_piece& Str() const { return m_Str; }
        double              Float() const { return m_Float; }
        double              AsFloat() const;
        void                SetFloat(float Val) { m_Float = Val; }
        int                 Int() const { return m_Integer; }
        void                SetInt(int Val) { m_Integer = Val; }
    private:
        string_piece        m_Str;
        Type                m_Type = TOK_INVALID;
        Location            m_Location;
        int                 m_Integer = 0;
        bool                m_Boolean = false;
        double              m_Float = 0.0;
    };
    class Tokenizer
    {
    public:
        static std::vector<Token> Run(const string_piece& str, const string_piece& filePath = "", Error* inErr = nullptr);
        ~Tokenizer();
    private:
        string_piece        m_File;
        string_piece        m_Source;
        std::vector<Token>  m_Tokens;
        int                 m_Line;
        int                 m_Column;
        int                 m_Offset;
        Error*              m_Err;
        bool                m_IsInHLSL;
        Token::Type         m_LastBlockToken;
        std::stack<Token::Type> m_BlockStack;
    private:
        explicit            Tokenizer(const string_piece& Source, const string_piece& Path, Error* inErr);
        std::vector<Token>  Run();
        Token::Type         ClassifyCurrent(bool& IsMultiLineComment);
        bool                CanAdvance();
        bool                CanRecede();
        bool                AtEnd();
        bool                HasErr();
        bool                Done();
        bool                IsCurrentStringTerminator(char quoteChar);
        char                CurChar();
        char                NextChar();
        Location            GetLocation();
        bool                GetLastToken(Token& Tok);
        void                SpawnError(const std::string& error, const std::string& help);

        void                Advance();
        void                AdvanceToEndOfToken(Location const& Loc, Token::Type& Type, int& iValue, double& fValue, bool isMultiLineComment = false);

        void                StripWhiteSpaces();
        void                MatchKeywords(Token::Type& Type, std::string const& word);
    };
    extern bool IsKeyword(string_piece const& Str);
    extern std::string Unquote(std::string const& inStr);
    extern string_piece Unquote(string_piece const& inStr);
}

namespace std
{
    template<>
    class hash<shaderlab::string_piece>
    {
    public:
        size_t operator()(shaderlab::string_piece const& str) const
        {
            size_t hash = 5381;
            int c;
            int i = 0;
            while (c = str[i] && i < str.length()) {
                hash = ((hash << 5) + hash) + c;
                i++;
            }
            return hash;
        }
    };
}