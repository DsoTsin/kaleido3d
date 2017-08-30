#include "sl_parser.h"

namespace shaderlab
{
    std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > Parser::s_PropertiesExpr = {
    { Token::TOK_COMMENT,       { nullptr, &Parser::Comment } },
    { Token::TOK_IDENTIFIER,    { nullptr, &Parser::Property } },
    { Token::TOK_LEFT_BRACKET,  { nullptr, &Parser::Property } },
    };

    std::unordered_map<Token::Type, std::pair<FnPrefix, FnInfix> > Parser::s_PropertyExpr = {
    { Token::TOK_PROP_INTEGER,      { nullptr, &Parser::PropertyInt } },
    { Token::TOK_PROP_FLOAT,        { nullptr, &Parser::PropertyFloat } },
    { Token::TOK_PROP_RANGE,        { nullptr, &Parser::PropertyRange } },
    { Token::TOK_PROP_COLOR,        { nullptr, &Parser::PropertyVector } },
    { Token::TOK_PROP_VECTOR,       { nullptr, &Parser::PropertyVector } },
    { Token::TOK_PROP_SAMPLER_1D,   { nullptr, &Parser::PropertyTexture } },
    { Token::TOK_PROP_SAMPLER_2D,   { nullptr, &Parser::PropertyTexture } },
    { Token::TOK_PROP_SAMPLER_3D,   { nullptr, &Parser::PropertyTexture } },
    { Token::TOK_PROP_SAMPLER_CUBE, { nullptr, &Parser::PropertyTexture } },
    };

    // IDENTIFIER(LITERAL, PROP_TYPE) ASSIGN (FLOAT|INT|LITERAL|VECTOR) NEWLINE
    // Or [ANNOTATION]IDENTIFIER(LITERAL, PROP_TYPE)
    NodePtr Parser::Property(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        ShaderNode* shaderNode = static_cast<ShaderNode*>(left.get());
        if (tok.GetType() != Token::TOK_IDENTIFIER && !CouldBeIdentifier(tok) 
            && tok.GetType() != Token::TOK_LEFT_BRACKET) {
            SpawnError("Parse property name failed", "");
            return left;
        }
        // parse name
        auto Name = tok.Str();
        // parse annotation 
        if (tok.GetType() == Token::TOK_LEFT_BRACKET) {
            while (!LookAhead(Token::TOK_RIGHT_BRACKET))
            {
                Consume();
            }
            Consume();
            auto Next = Consume();
            if (!CouldBeIdentifier(Next) && Next.GetType() != Token::TOK_IDENTIFIER) {
                SpawnError("Parse property failed", "Expected 'IDENTIFIER' here");
                return left;
            }
            Name = Next.Str();
        }
        // left has to be shadernode
        // new Property
        if (!Match(Token::TOK_LEFT_PAREN)) {
            SpawnError("Parse property failed", "Expected '(' here");
            return left;
        }
        auto DisplayName = Cur().Str();
        if (!Match(Token::TOK_LITERAL_STRING)) {
            SpawnError("Parse property failed", "Expected '\"Literal\"' here");
            return left;
        }
        if (!Match(Token::TOK_COMMA)) {
            SpawnError("Parse property failed", "Expected ',' here");
            return left;
        }
        if (!IsProperty(Cur())) {
            SpawnError("Parse property failed", "Expected '1D', '2D', '3D', 'Float', 'Int', 'UInt', 'Vector', 'Color', 'Cube' here");
            return left;
        }
        Token::Type Type = Cur().GetType();
        auto propNode = std::make_unique<PropertyNode>();
        propNode->SetName(Name.to_string());
        propNode->SetDisplayName(Unquote(DisplayName).to_string());
        propNode->SetAnnotation(Name.to_string());
        FnInfix infix = s_PropertyExpr[Type].second;
        if (infix) {
            auto node = (this->*infix)(std::move(propNode), Consume());
            if (node) {
                shaderNode->AddProperty(std::move(node));
            }
        } else { // Cannot Accept
            SpawnError("Parsing property failed", "Unknow property type");
        }
        return left;
    }

    NodePtr Parser::PropertyRange(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        if (!Match(Token::TOK_LEFT_PAREN)) { // (
            SpawnError("Parse range property failed", "Expected '(' here");
            return left;
        }
        if (!IsNumber(Cur())) { // Number
            SpawnError("Parse range property failed", "Expected Number here");
            return left;
        }
        auto Min = Consume();
        if (!Match(Token::TOK_COMMA)) { // ,
            SpawnError("Parse range property failed", "Expected ',' here");
            return left;
        }
        if (!IsNumber(Cur())) { // Number
            SpawnError("Parse range property failed", "Expected Number here");
            return left;
        }
        auto Max = Consume();
        if (!Match(Token::TOK_RIGHT_PAREN)) { // )
            SpawnError("Parse range property failed", "Expected ')' here");
            return left;
        }
        if (!Match(Token::TOK_RIGHT_PAREN)) { // )
            SpawnError("Parse range property failed", "Expected ')' here");
            return left;
        }
        if (!Match(Token::TOK_ASSIGN)) { // = 
            SpawnError("Parse range property failed", "Expected '=' here");
            return left;
        }

        if (!IsNumber(Cur())) { // Number
            SpawnError("Parse range property value failed", "Expected Number here");
            return left;
        }
        auto Val = Consume();
        PropertyNode* propNode = static_cast<PropertyNode*>(left.get());
        if (Min.GetType() == Token::TOK_FLOAT || Max.GetType() == Token::TOK_FLOAT
            || Val.GetType() == Token::TOK_FLOAT) // Float Range
        {
            propNode->Construct(Token::TOK_FLOAT);
            propNode->SetFloatWithRange(Val.AsFloat(), Min.AsFloat(), Max.AsFloat());
        } else { // Int Range
            propNode->Construct(Token::TOK_INTEGER);
            propNode->SetIntWithRange(Val.Int(), Min.Int(), Max.Int());
        }

        return left;
    }

    NodePtr Parser::PropertyInt(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        if (!Match(Token::TOK_RIGHT_PAREN)) {
            SpawnError("Parse integer property failed", "Expected ')' here");
            return left;
        }
        if (!Match(Token::TOK_ASSIGN)) {
            SpawnError("Parse integer property failed", "Expected '=' here");
            return left;
        }
        auto CurTok = Cur();
        if (!Match(Token::TOK_INTEGER)) { // Int
            SpawnError("Parse integer property failed", "Expected 'FLOAT' here");
            return left;
        }
        PropertyNode* propNode = static_cast<PropertyNode*>(left.get());
        propNode->Construct(tok.GetType());
        propNode->SetInt(tok.Int());
        return left;
    }

    NodePtr Parser::PropertyFloat(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        if (!Match(Token::TOK_RIGHT_PAREN)) { // )
            SpawnError("Parse float property failed", "Expected ')' here");
            return left;
        }
        if (!Match(Token::TOK_ASSIGN)) { // = 
            SpawnError("Parse float property failed", "Expected '=' here");
            return left;
        }
        auto CurTok = Cur();
        if (!Match(Token::TOK_FLOAT)) { // Float
            SpawnError("Parse float property failed", "Expected 'FLOAT' here");
            return left;
        }
        PropertyNode* propNode = static_cast<PropertyNode*>(left.get());
        propNode->Construct(tok.GetType());
        propNode->SetFloat(tok.Float());
        return left;
    }

    NodePtr Parser::PropertyVector(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        PropertyNode* propNode = static_cast<PropertyNode*>(left.get());
        propNode->Construct(tok.GetType());
        if (!Match(Token::TOK_RIGHT_PAREN)) { // )
            SpawnError("Parse vector property failed", "Expected ')' here");
            return left;
        }
        if (!Match(Token::TOK_ASSIGN)) { // = 
            SpawnError("Parse vector property failed", "Expected '=' here");
            return left;
        }

        if (!Match(Token::TOK_LEFT_PAREN)) { // (
            SpawnError("Parse vector property failed", "Expected ')' here");
            return left;
        }
        // Vector Number Here
        Token Num;
        if (!ConsumeNumber(Num)) {
            SpawnError("Parse vector property failed", "Expected 'NUMBER' here");
            return left;
        }
        
        if (!Match(Token::TOK_COMMA)) { // ,
            SpawnError("Parse vector property failed", "Expected ',' here");
            return left;
        }
        if (!ConsumeNumber(Num)) {
            SpawnError("Parse vector property failed", "Expected 'NUMBER' here");
            return left;
        }
        if (!Match(Token::TOK_COMMA)) { // ,
            SpawnError("Parse vector property failed", "Expected ',' here");
            return left;
        }
        if (!ConsumeNumber(Num)) {
            SpawnError("Parse vector property failed", "Expected 'NUMBER' here");
            return left;
        }

        // Next ) or ,
        if (LookAhead(Token::TOK_COMMA)) {
            Consume();

            if (!ConsumeNumber(Num)) {
                SpawnError("Parse vector property failed", "Expected 'NUMBER' here");
                return left;
            }

        }

        if (!Match(Token::TOK_RIGHT_PAREN)) { // )
            SpawnError("Parse vector property failed", "Expected ')' here");
            return left;
        }

        return left;
    }

    NodePtr Parser::PropertyTexture(std::unique_ptr<ParserNode> left, const Token & tok)
    {
        PropertyNode* propNode = static_cast<PropertyNode*>(left.get());
        propNode->Construct(tok.GetType());
        if (!Match(Token::TOK_RIGHT_PAREN)) { // )
            SpawnError("Parse texture property failed", "Expected ')' here");
            return left;
        }
        if (!Match(Token::TOK_ASSIGN)) { // = 
            SpawnError("Parse texture property failed", "Expected '=' here");
            return left;
        }
        auto CurTok = Cur();
        if (!Match(Token::TOK_LITERAL_STRING)) { // "white"
            SpawnError("Parse texture property failed", "Expected '\"STRING\"' here");
            return left;
        }
        propNode->SetTextureName(CurTok.Str().to_string());
        if (!Match(Token::TOK_LEFT_BRACE)) { // {
            SpawnError("Parse texture property failed", "Expected '{' here");
            return left;
        }
        if (!Match(Token::TOK_RIGHT_BRACE)) { // {
            SpawnError("Parse texture property failed", "Expected '}' here");
            return left;
        }
        return left;
    }

}