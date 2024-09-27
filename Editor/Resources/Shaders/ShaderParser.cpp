#include "ShaderParser.hpp"
#include "ShaderParser.hpp"

#include <spdlog/spdlog.h>
#include <sstream>

#define TokenEOFCheckExt(context) \
    if ((state.Tokens.size() < state.Current)) \
    { \
        logger->error("[{}]: Unexpected end for token stream for operation " context ":{}!", state.Tokens.end()->Line, __LINE__); \
        state.ErrorRaised = true; \
        continue; \
    }

#define TokenEOFCheck(context) \
    if ((state.Tokens.size() < state.Current)) \
    { \
        logger->error("[{}]: Unexpected end for token stream for operation " context ":{}!", state.Tokens.end()->Line, __LINE__); \
        state.ErrorRaised = true; \
        continue; \
    }

#define TokenEOFCheckSpd(context) \
    if ((state.Tokens.size() < state.Current)) \
    { \
        spdlog::get("ED")->error("[{}]: Unexpected end for token stream for operation " context ":{}!", state.Tokens.end()->Line, __LINE__); \
        state.ErrorRaised = true; \
        break; \
    }

const std::map<std::string_view, ed::ShaderTokenType> s_Keywords = {
    { "ConstantBuffer", ed::ShaderTokenType::ConstantBuffer },
    { "StructuredBuffer", ed::ShaderTokenType::StructuredBuffer },
    { "SamplerState", ed::ShaderTokenType::SamplerState },
    { "StaticSamplerState", ed::ShaderTokenType::StaticSamplerState },
    { "Texture1D", ed::ShaderTokenType::Texture1D },
    { "Texture2D", ed::ShaderTokenType::Texture2D },
    { "Texture3D", ed::ShaderTokenType::Texture3D },
    { "Texture1DArray", ed::ShaderTokenType::Texture1DArray },
    { "Texture2DArray", ed::ShaderTokenType::Texture2DArray },
    { "Texture3DArray", ed::ShaderTokenType::Texture3DArray },
    { "float", ed::ShaderTokenType::Float },
    { "float2", ed::ShaderTokenType::Float2 },
    { "float3", ed::ShaderTokenType::Float3 },
    { "float4", ed::ShaderTokenType::Float4 },
    { "float4x4", ed::ShaderTokenType::Float4x4 },
    { "matrix", ed::ShaderTokenType::Float4x4 },
    { "uint", ed::ShaderTokenType::UInt },
    { "uint2", ed::ShaderTokenType::UInt2 },
    { "uint3", ed::ShaderTokenType::UInt3 },
    { "uint4", ed::ShaderTokenType::UInt4 },
    { "byte4", ed::ShaderTokenType::Byte4 },
    { "struct", ed::ShaderTokenType::Struct },
    { "return", ed::ShaderTokenType::Return },
};

const std::vector<ed::ShaderTokenType> s_ValueKeywords = {
    ed::ShaderTokenType::Float,
    ed::ShaderTokenType::Float2,
    ed::ShaderTokenType::Float3,
    ed::ShaderTokenType::Float4,
    ed::ShaderTokenType::Float4x4,
    ed::ShaderTokenType::UInt,
    ed::ShaderTokenType::UInt2,
    ed::ShaderTokenType::UInt3,
    ed::ShaderTokenType::UInt4,
    ed::ShaderTokenType::Byte4,
};

const std::vector<std::string_view> s_Semantics = {
    "POSITION",
    "TEXCOORD",
    "TEXCOORD0",
    "TEXCOORD1",
    "TEXCOORD2",
    "TEXCOORD3",
    "TEXCOORD4",
    "TEXCOORD5",
    "TEXCOORD6",
    "TEXCOORD7",
    "COLOR",
    "COLOR0",
    "COLOR1",
    "COLOR2",
    "COLOR3",
    "COLOR4",
    "COLOR5",
    "COLOR6",
    "COLOR7",
    "NORMAL",
    "NORMAL0",
    "NORMAL1",
    "NORMAL2",
    "NORMAL3",
    "NORMAL4",
    "NORMAL5",
    "NORMAL6",
    "NORMAL7",
};

const std::map<uint8_t, std::string_view> s_TextureDimensions = {
    { (uint8_t)ed::ShaderTokenType::Texture1D, "Texture1D" },
    { (uint8_t)ed::ShaderTokenType::Texture1DArray, "Texture1DArray" },
    { (uint8_t)ed::ShaderTokenType::Texture2D, "Texture2D" },
    { (uint8_t)ed::ShaderTokenType::Texture2DArray, "Texture2DArray" },
    { (uint8_t)ed::ShaderTokenType::Texture3D, "Texture3D" },
    { (uint8_t)ed::ShaderTokenType::Texture3DArray, "Texture3DArray" },
};

void ed::ShaderParser::InsertTokenAt(LocalState& state, ShaderTokenType type)
{
    state.Tokens.push_back(ShaderToken{
        .Type = type,
        .FloatValue = 0.0f,
        .Snippet = pge::SmallString(&state.Source[state.Start], (state.Current - state.Start)),
        .Line = state.Scan_Line
        });
}

void ed::ShaderParser::InsertTokenAt(LocalState& state, ShaderTokenType type, std::string_view snippet)
{
    state.Tokens.push_back(ShaderToken {
        .Type = type,
        .FloatValue = 0.0f,
        .Snippet = std::string(snippet.data(), snippet.size()),
        .Line = state.Scan_Line
        });
}

void ed::ShaderParser::InsertTokenAt(LocalState& state, ShaderTokenType type, float v)
{
    state.Tokens.push_back(ShaderToken{
        .Type = type,
        .FloatValue = v,
        .Snippet = pge::SmallString(&state.Source[state.Start], (state.Current - state.Start)),
        .Line = state.Scan_Line
        });
}

void ed::ShaderParser::InsertTokenAt(LocalState& state, ShaderTokenType type, uint32_t v)
{
    state.Tokens.push_back(ShaderToken{
        .Type = type,
        .UIntValue = v,
        .Snippet = pge::SmallString(&state.Source[state.Start], (state.Current - state.Start)),
        .Line = state.Scan_Line
        });
}

void ed::ShaderParser::Identifier(LocalState& state)
{
    while (isalnum(state.Source[state.Current]) || state.Source[state.Current] == '_') state.Current++;

    std::string_view snippet = std::string_view(&state.Source[state.Start], state.Current - state.Start);
    if (s_Keywords.count(snippet))
        InsertTokenAt(state, (ShaderTokenType)s_Keywords.find(snippet)->second);
    else
        InsertTokenAt(state, ed::ShaderTokenType::Identifier, snippet);
}

void ed::ShaderParser::Number(LocalState& state)
{
    bool hasDigit = false;
    while (isdigit(state.Source[state.Current]) || state.Source[state.Current] == '.')
    {
        if (!hasDigit && state.Source[state.Current] == '.')
            hasDigit = true;
        state.Current++;
    }

    std::string_view snippet = std::string_view(&state.Source[state.Start], state.Current - state.Start);
    if (state.Source[state.Current] == 'f')
    {
        float v = 0.0f;
        auto result = std::from_chars(snippet.data(), snippet.data() + snippet.size(), v);

        if (result.ec == std::errc::invalid_argument)
        {
            spdlog::get("ED")->error("[{}]: Failed to parse number value: \"{}\"", state.Scan_Line, snippet);
            state.ErrorRaised = true;
            return;
        }

        InsertTokenAt(state, ShaderTokenType::Number, v);

        state.Current++;
    }
    else
    {
        uint32_t v = 0;
        auto result = std::from_chars(snippet.data(), snippet.data() + snippet.size(), v);

        if (result.ec == std::errc::invalid_argument)
        {
            spdlog::get("ED")->error("[{}]: Failed to parse integral value: \"{}\"", state.Scan_Line, snippet);
            state.ErrorRaised = true;
            return;
        }

        InsertTokenAt(state, ShaderTokenType::Integral, v);
    }
}

void ed::ShaderParser::String(LocalState& state, char delim)
{
    while (state.Source[state.Current] != delim) state.Current++;
    state.Current++;

    std::string_view snippet = std::string_view(&state.Source[state.Start + 1], state.Current - state.Start - 2);
    InsertTokenAt(state, ShaderTokenType::String, snippet);
}

pge::TextureFilter ed::ShaderParser::StringConvertFilter(std::string_view string) noexcept
{
    if (string == "Linear") return pge::TextureFilter::Linear;
    else if (string == "Point") return pge::TextureFilter::Point;
    return pge::TextureFilter::Unkown;
}

pge::AddressMode ed::ShaderParser::StringConvertAddress(std::string_view string) noexcept
{
    if (string == "Wrap") return pge::AddressMode::Wrap;
    else if (string == "Repeat") return pge::AddressMode::Repeat;
    else if (string == "Mirror") return pge::AddressMode::Mirror;
    return pge::AddressMode::Unkown;
}

bool ed::ShaderParser::ExamineAllStructs(LocalState& state, std::vector<std::pair<pge::SmallString, uint32_t>>& indices)
{
    auto logger = spdlog::get("ED");

    state.Start = 0;
    state.Current = state.Result.Bounds.Start;

    while (state.Current < state.Result.Bounds.End)
    {
        ShaderToken& token = state.Tokens[state.Current++];
        if (token.Type == ShaderTokenType::Struct)
        {
            TokenEOFCheck("Struct");

            ShaderToken& name = state.Tokens[state.Current++];
            if (name.Type != ShaderTokenType::Identifier)
            {
                logger->error("[{}]: Expected identifier for struct definition! Got: {}", token.Line, (uint8_t)name.Type);
                state.ErrorRaised = true;
                continue;
            }

            indices.push_back(std::make_pair(name.Snippet, state.Current));
        }
    }

    return !state.ErrorRaised;
}

bool ed::ShaderParser::ASTIsFunction(LocalState&, std::string_view name)
{
    return false;
}

void ed::ShaderParser::ASTStruct(LocalState& state)
{
    ShaderToken& name = state.Tokens[state.Current++];
    if (name.Type != ShaderTokenType::Identifier)
    {
        spdlog::get("ED")->error("[{}]: Expected name after struct decleration!", name.Line);
        state.ErrorRaised = true;
        return;
    }

    {
        ShaderToken& check = state.Tokens[state.Current++];
        if (check.Type != ShaderTokenType::OpenCurlyBracket)
        {
            spdlog::get("ED")->error("[{}]: Expected body definition after struct decleration!", check.Line);
            state.ErrorRaised = true;
            return;
        }
    }

    ShaderTreeNode node{};
    node.Name = name.Snippet;
    node.Type = "Struct";

    while (state.Current < state.Tokens.size() && state.Tokens[state.Current].Type != ShaderTokenType::CloseCurlyBracket)
    {
        ShaderToken& varType = state.Tokens[state.Current++];
        if (!std::count(s_ValueKeywords.begin(), s_ValueKeywords.end(), varType.Type))
        {
            spdlog::get("ED")->error("[{}]: Expected a generic value type for struct member!", varType.Line);
            state.ErrorRaised = true;
            continue;
        }

        ShaderToken& varName = state.Tokens[state.Current++];
        if (varName.Type != ShaderTokenType::Identifier)
        {
            spdlog::get("ED")->error("[{}]: Expected a name after variable value type decleration!", varName.Line);
            state.ErrorRaised = true;
            continue;
        }

        ShaderTreeNode treeNode{};
        treeNode.Name = varName.Snippet;
        treeNode.Type = "Field";
        treeNode.Attributes.insert(std::make_pair("Type", varType.Snippet));

        if (state.Tokens[state.Current].Type == ShaderTokenType::Colon)
        {
            state.Current++;

            ShaderToken& semantic = state.Tokens[state.Current++];
            if (semantic.Type != ShaderTokenType::Identifier)
            {
                spdlog::get("ED")->error("[{}]: Expected a semantic decleration after variable because of colon designator!", semantic.Line);
                state.ErrorRaised = true;
                continue;
            }

            treeNode.Attributes.insert(std::make_pair("Semantic", semantic.Snippet));
        }

        {
            ShaderToken& check = state.Tokens[state.Current++];
            if (check.Type != ShaderTokenType::Semicolon)
            {
                spdlog::get("ED")->error("[{}]: Expected a segment terminator at end of variable decleration!", check.Line);
                state.ErrorRaised = true;
                return;
            }
        }

        node.Children.push_back(treeNode);
    }

    state.Current++;

    {
        ShaderToken& check = state.Tokens[state.Current++];
        if (check.Type != ShaderTokenType::Semicolon)
        {
            spdlog::get("ED")->error("[{}]: Expected a body terminator at end of struct decleration!", check.Line);
            state.ErrorRaised = true;
            return;
        }
    }

    state.Result.AST.Children.push_back(node);
}

void ed::ShaderParser::ASTConstantBuffer(LocalState& state)
{
    {
        ShaderToken& check = state.Tokens[state.Current++];
        if (check.Type != ShaderTokenType::Lessthan)
        {
            spdlog::get("ED")->error("[{}]: Expected type specifier for constant buffer!", check.Line);
            state.ErrorRaised = true;
            return;
        }
    }

    ShaderToken& valueType = state.Tokens[state.Current++];
    if (valueType.Type != ShaderTokenType::Identifier || !std::count_if(state.Result.CBParams.begin(), state.Result.CBParams.end(), [&](ShaderCBParam& param) { return param.ValueType == valueType.Snippet.c_str(); }))
    {
        spdlog::get("ED")->error("[{}]: Unable to resolve constant buffer type!", valueType.Line);
        state.ErrorRaised = true;
        return;
    }

    {
        ShaderToken& check = state.Tokens[state.Current++];
        if (check.Type != ShaderTokenType::Greaterthan)
        {
            spdlog::get("ED")->error("[{}]: Expected type closer for constant buffer!", check.Line);
            state.ErrorRaised = true;
            return;
        }
    }

    ShaderToken& name = state.Tokens[state.Current++];
    if (name.Type != ShaderTokenType::Identifier)
    {
        spdlog::get("ED")->error("[{}]: Expected name for constant buffer decleration!", name.Line);
        state.ErrorRaised = true;
        return;
    }

    ShaderTreeNode node{};
    node.Name = name.Snippet;
    node.Type = "ConstantBuffer";
    node.Attributes.insert(std::make_pair("Type", valueType.Snippet));

    state.Result.AST.Children.push_back(node);
}

void ed::ShaderParser::ASTSamplerState(LocalState& state)
{
    ShaderToken& name = state.Tokens[state.Current++];
    if (name.Type != ShaderTokenType::Identifier)
    {
        spdlog::get("ED")->error("[{}]: Expected name for SamplerState decleration!", name.Line);
        state.ErrorRaised = true;
        return;
    }

    ShaderTreeNode node{};
    node.Name = name.Snippet;
    node.Type = "SamplerState";

    state.Result.AST.Children.push_back(node);
}

void ed::ShaderParser::ASTStaticSamplerState(LocalState& state)
{
    ShaderToken& name = state.Tokens[state.Current++];
    if (name.Type != ShaderTokenType::Identifier)
    {
        spdlog::get("ED")->error("[{}]: Expected name for StaticSamplerState decleration!", name.Line);
        state.ErrorRaised = true;
        return;
    }

    {
        ShaderToken& check = state.Tokens[state.Current++];
        if (check.Type != ShaderTokenType::OpenCurlyBracket)
        {
            spdlog::get("ED")->error("[{}]: Expected body definition for StaticSamplerState decleration!", check.Line);
            state.ErrorRaised = true;
            return;
        }
    }

    ShaderTreeNode node{};
    node.Name = name.Snippet;
    node.Type = "StaticSamplerState";

    //should already be accepted and in an "ok" state because of previous steps but consistency and safety somewhat first!
    while (state.Current < state.Tokens.size() && state.Tokens[state.Current].Type != ShaderTokenType::CloseCurlyBracket)
    {
        ShaderToken& fieldName = state.Tokens[state.Current++];
        if (fieldName.Type != ShaderTokenType::Identifier)
        {
            spdlog::get("ED")->error("[{}]: Expected field identifier in StaticSamplerState body!", fieldName.Line);
            state.ErrorRaised = true;
            continue;
        }

        {
            ShaderToken& check = state.Tokens[state.Current++];
            if (check.Type != ShaderTokenType::Equals)
            {
                spdlog::get("ED")->error("[{}]: Expected associated value for StaticSamplerState field!", check.Line);
                state.ErrorRaised = true;
                return;
            }
        }

        ShaderToken& fieldValue = state.Tokens[state.Current++];
        if (fieldValue.Type != ShaderTokenType::String)
        {
            spdlog::get("ED")->error("[{}]: StaticSamplerState field value must be a string!", fieldValue.Line);
            state.ErrorRaised = true;
            continue;
        }

        ShaderTreeNode treeNode{};
        treeNode.Name = fieldName.Snippet;
        treeNode.Type = "Field";
        treeNode.Attributes.insert(std::make_pair("Value", fieldValue.Snippet));

        node.Children.push_back(treeNode);
    }

    state.Current++;

    state.Result.AST.Children.push_back(node);
}

void ed::ShaderParser::ASTTexture(LocalState& state, ShaderToken& token)
{
    ShaderToken& name = state.Tokens[state.Current++];
    if (name.Type != ShaderTokenType::Identifier)
    {
        spdlog::get("ED")->error("[{}]: Expected name for {} decleration!", name.Line, token.Snippet.c_str());
        state.ErrorRaised = true;
        return;
    }

    ShaderTreeNode node{};
    node.Name = name.Snippet;
    node.Type = token.Snippet;

    state.Result.AST.Children.push_back(node);
}

void ed::ShaderParser::ASTFunction(LocalState& state, ShaderToken& valueType)
{
    ShaderToken& name = state.Tokens[state.Current++];
    if (name.Type != ShaderTokenType::Identifier)
    {
        spdlog::get("ED")->error("[{}]: Expected name for function!", name.Line);
        state.ErrorRaised = true;
        return;
    }

    {
        ShaderToken& check = state.Tokens[state.Current++];
        if (check.Type != ShaderTokenType::OpenBracket)
        {
            spdlog::get("ED")->error("[{}]: Expected open bracket for function arguments!", check.Line);
            state.ErrorRaised = true;
            return;
        }
    }

    ShaderTreeNode node{};
    node.Name = name.Snippet;
    node.Type = "Function";
    node.Attributes.insert(std::make_pair("Return", valueType.Snippet));

    while (state.Current < state.Tokens.size() && state.Tokens[state.Current].Type != ShaderTokenType::CloseBracket)
    {
        ShaderToken& argType = state.Tokens[state.Current++];
        if (argType.Type != ShaderTokenType::Identifier)
        {
            spdlog::get("ED")->error("[{}]: Expected type for function argument!", argType.Line);
            state.ErrorRaised = true;
            return;
        }

        ShaderToken& argName = state.Tokens[state.Current++];
        if (argName.Type != ShaderTokenType::Identifier)
        {
            spdlog::get("ED")->error("[{}]: Expected name for function argument!", argName.Line);
            state.ErrorRaised = true;
            return;
        }

        ShaderTreeNode treeNode{};
        treeNode.Name = argName.Snippet;
        treeNode.Type = "Argument";
        treeNode.Attributes.insert(std::make_pair("Type", argType.Snippet));

        node.Children.push_back(treeNode);
    }

    {
        ShaderToken& check = state.Tokens[state.Current++];
        if (check.Type != ShaderTokenType::CloseBracket)
        {
            spdlog::get("ED")->error("[{}]: Expected closing bracket for function arguments!", check.Line);
            state.ErrorRaised = true;
            return;
        }
    }

    {
        ShaderToken& check = state.Tokens[state.Current++];
        if (check.Type != ShaderTokenType::OpenCurlyBracket)
        {
            spdlog::get("ED")->error("[{}]: Expected body decleration for function!", check.Line);
            state.ErrorRaised = true;
            return;
        }
    }

    ShaderTreeNode body{};
    body.Type = "Body";
    node.Children.push_back(body);

    state.AST_Active = &node.Children.back();
    state.Result.AST.Children.push_back(node);
}

void ed::ShaderParser::ASTCall(LocalState& state, ShaderToken& function)
{
}

void ed::ShaderParser::ASTVariable(LocalState& state, ShaderToken& valueType)
{
}

bool ed::ShaderParser::ExamineAllFunctions(LocalState& state, std::vector<ShaderFunctionDefinition>& definitions)
{
    state.Start = 0;
    state.Current = 0;

    while (state.Current < state.Tokens.size())
    {
        state.Start = state.Current;
        ShaderToken& token = state.Tokens[state.Current++];

        switch (token.Type)
        {
        case ShaderTokenType::Float:
        case ShaderTokenType::Float2:
        case ShaderTokenType::Float3:
        case ShaderTokenType::Float4:
        case ShaderTokenType::Float4x4:
        case ShaderTokenType::UInt:
        case ShaderTokenType::UInt2:
        case ShaderTokenType::UInt3:
        case ShaderTokenType::UInt4:
        case ShaderTokenType::Identifier:
        {
            if (state.Tokens[state.Current + 1].Type != ShaderTokenType::OpenBracket) continue;

            ShaderFunctionDefinition definition{};
            definition.LocalStart = state.Current;

            ShaderToken& name = state.Tokens[state.Current++];
            if (name.Type != ShaderTokenType::Identifier)
            {
                spdlog::get("ED")->error("[{}]: Expected name for function!", name.Line);
                state.ErrorRaised = true;
                continue;
            }

            //already checked
            // args: '('


            while (state.Current < state.Tokens.size() && state.Tokens[state.Current].Type != ShaderTokenType::OpenCurlyBracket) state.Current++;
            definition.Start = ++state.Current;

            while (state.Current < state.Tokens.size() && state.Tokens[state.Current].Type != ShaderTokenType::CloseCurlyBracket) state.Current++;
            definition.End = state.Current;

            definitions.push_back(definition);
            break;
        }
        default:
            break;
        }
    }

    return !state.ErrorRaised;
}

void ed::ShaderParser::PrintDebug(LocalState& state, ShaderTreeNode& node, uint32_t depth)
{
    auto logger = spdlog::get("ED");

    std::vector<char> vector;
    vector.resize(depth*2, ' ');

    logger->info("{} {}", vector.empty() ? "" : vector.data(), node.Name.c_str() == nullptr ? "ROOT" : node.Name.c_str());
    for (auto& attrib : node.Attributes)
    {
        logger->info(" {} {} : {}", vector.data(), attrib.first.data(), attrib.second.c_str());
    }

    depth++;
    for (ShaderTreeNode& child : node.Children)
    {
        PrintDebug(state, child, depth);
    }
}

bool ed::ShaderParser::PerformScan(LocalState& state)
{
    auto logger = spdlog::get("ED");

    state.Scan_Line++;
    while (state.Current < state.Source.size())
    {
        state.Start = state.Current;
        char ch = state.Source[state.Current++];

        switch (ch)
        {
        case '+': InsertTokenAt(state, ShaderTokenType::Plus); break;
        case '-': InsertTokenAt(state, ShaderTokenType::Minus); break;
        case '/': InsertTokenAt(state, ShaderTokenType::Slash); break;
        case '*': InsertTokenAt(state, ShaderTokenType::Star); break;

        case '(': InsertTokenAt(state, ShaderTokenType::OpenBracket); break;
        case ')': InsertTokenAt(state, ShaderTokenType::CloseBracket); break;
        case '[': InsertTokenAt(state, ShaderTokenType::OpenSquareBracket); break;
        case ']': InsertTokenAt(state, ShaderTokenType::CloseSquareBracket); break;
        case '{': InsertTokenAt(state, ShaderTokenType::OpenCurlyBracket); break;
        case '}': InsertTokenAt(state, ShaderTokenType::CloseCurlyBracket); break;

        case ':': InsertTokenAt(state, ShaderTokenType::Colon); break;
        case ';': InsertTokenAt(state, ShaderTokenType::Semicolon); break;
        case '.': InsertTokenAt(state, ShaderTokenType::Comma); break;
        case ',': InsertTokenAt(state, ShaderTokenType::Period); break;
        case '<': InsertTokenAt(state, ShaderTokenType::Lessthan); break;
        case '>': InsertTokenAt(state, ShaderTokenType::Greaterthan); break;
        case '=': InsertTokenAt(state, ShaderTokenType::Equals); break;

        case '"':
        case '\'': String(state, ch); break;

        case '\n': state.Scan_Line++;
        case '\t':
        case '\r': break;

        default:
            if (isblank(ch)) break;
            else if (isdigit(ch)) { Number(state); break; }
            else if (isalpha(ch) || ch == '_') { Identifier(state); break; }
            logger->error("[{}]: Unkown token at: {}", state.Scan_Line, ch);
            state.ErrorRaised = true;
            break;
        }
    }

    return !state.ErrorRaised;
}

bool ed::ShaderParser::PerformBounds(LocalState& state)
{
    auto logger = spdlog::get("ED");

    state.Start = 0;
    state.Current = 0;
    while (state.Current < state.Tokens.size())
    {
        state.Start = state.Current;
        ShaderToken& token = state.Tokens[state.Current++];

        if (token.Type != ShaderTokenType::Identifier)
        {
            logger->error("[{}]: Non-identifier type found outside of valid area!", token.Line);
            state.ErrorRaised = true;
            continue;
        }

        TokenEOFCheckExt("Global");

        {
            ShaderToken& check = state.Tokens[state.Current++];
            if (check.Type != ShaderTokenType::Equals)
            {
                logger->error("[{}]: No value associated with global shader definition!", check.Line);
                state.ErrorRaised = true;
                continue;
            }
        }

        TokenEOFCheckExt("Global");

        {
            ShaderToken& check = state.Tokens[state.Current++];
            if (check.Type != ShaderTokenType::OpenCurlyBracket)
            {
                logger->error("[{}]: Body definition missing from global shader definition!", check.Line);
                state.ErrorRaised = true;
                continue;
            }
        }

        TokenEOFCheckExt("Global");

        if (token.Snippet == "Tags")
        {
            ShaderToken dummy{};
            ShaderToken& local = dummy; //for some reason THIS deletes the orignal string by zero-ing it out
            while (state.Current < state.Tokens.size() && (local = state.Tokens[state.Current++]).Type != ShaderTokenType::CloseCurlyBracket)
            {
                if (local.Type != ShaderTokenType::Identifier)
                {
                    logger->error("[{}]: Shader definition child field needs to be of type \"Identifier\"! Got: \"{}\"", local.Line, local.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }

                TokenEOFCheckExt("Tags");

                {
                    ShaderToken& check = state.Tokens[state.Current++];
                    if (check.Type != ShaderTokenType::Equals)
                    {
                        logger->error("[{}]: No value associated with child field definition!", check.Line);
                        state.ErrorRaised = true;
                        continue;
                    }
                }

                TokenEOFCheckExt("Tags");

                ShaderToken& value = state.Tokens[state.Current++];
                if (value.Type != ShaderTokenType::String)
                {
                    logger->error("[{}]: Shader definition child field value needs to be of type \"String\"! Got: \"{}\"", local.Line, local.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }

                state.Result.Tags.Values.insert(std::make_pair(local.Snippet, value.Snippet));
            }

            if (state.Tokens.size() < state.Current)
            {
                logger->error("[{}]: Unexpected end for global shader definition: {}!", state.Tokens.end()->Line, __LINE__);
                state.ErrorRaised = true;
                continue;
            }
        }
        else if (token.Snippet == "ShaderSource")
        {
            state.Start = state.Current;

            uint32_t bodyCount = 1;
            while (state.Current < state.Tokens.size() && bodyCount > 0)
            {
                ShaderToken& local = state.Tokens[state.Current++];
                if (local.Type == ShaderTokenType::OpenCurlyBracket)
                    bodyCount++;
                else if (local.Type == ShaderTokenType::CloseCurlyBracket)
                    bodyCount--;
            }

            if (state.Tokens.size() < state.Current)
            {
                logger->error("[{}]: Unexpected end for global shader definition: {}!", state.Tokens.end()->Line, __LINE__);
                state.ErrorRaised = true;
                continue;
            }

            state.Result.Bounds = { .Start = state.Start, .End = state.Current - 1 };
        }
    }

    return !state.ErrorRaised;
}

bool ed::ShaderParser::PerformReflect(LocalState& state)
{
    auto logger = spdlog::get("ED");

    std::vector<std::pair<pge::SmallString, uint32_t>> structIndices{};
    if (!ExamineAllStructs(state, structIndices))
    {
        logger->error("Failed to examine struct indices in shader for reflection!");
        state.ErrorRaised = true;
    }

    state.Start = 0;
    state.Current = state.Result.Bounds.Start;
    while (state.Current < state.Result.Bounds.End)
    {
        state.Start = state.Current;
        ShaderToken& token = state.Tokens[state.Current++];

        switch (token.Type)
        {
        case ShaderTokenType::Identifier:
        {
            bool breakSwitch = true;
            for (auto& pair : structIndices)
                if (pair.first == token.Snippet)
                {
                    breakSwitch = false;
                    break;
                }

            if (breakSwitch)
                break;
        }
        case ShaderTokenType::Float:
        case ShaderTokenType::Float2:
        case ShaderTokenType::Float3:
        case ShaderTokenType::Float4:
        case ShaderTokenType::UInt:
        case ShaderTokenType::UInt2:
        case ShaderTokenType::UInt3:
        case ShaderTokenType::UInt4:
        {
            if (state.Tokens[state.Current + 1].Type != ShaderTokenType::OpenBracket) break;

            ShaderToken& name = state.Tokens[state.Current++];
            if (name.Type != ShaderTokenType::Identifier)
            {
                logger->error("[{}]: Expected a name for function decleration!", name.Line);
                state.ErrorRaised = true;
                break;
            }

            if (name.Snippet != "vertex") break;

            {
                ShaderToken& check = state.Tokens[state.Current++];
                if (check.Type != ShaderTokenType::OpenBracket)
                {
                    logger->error("[{}]: Expected opening bracket for function arguments!", token.Line);
                    state.ErrorRaised = true;
                    continue;
                }
            }

            ShaderToken& targetValueType = state.Tokens[state.Current++];
            if (targetValueType.Type != ShaderTokenType::Identifier)
            {
                logger->error("[{}]: Expected name for vertex input struct!", name.Line);
                state.ErrorRaised = true;
                break;
            }

            bool didFind = false;
            for (auto& pair : structIndices)
            {
                if (pair.first == targetValueType.Snippet)
                {
                    uint32_t startToken = pair.second + 1;
                    while (startToken < state.Tokens.size() && state.Tokens[startToken].Type != ShaderTokenType::CloseCurlyBracket)
                    {
                        ShaderToken& fieldType = state.Tokens[startToken++];
                        if (!std::count(s_ValueKeywords.begin(), s_ValueKeywords.end(), fieldType.Type))
                        {
                            logger->error("[{}]: Expected generic value type for variable decleration! Got: \"{}\"", fieldType.Line, fieldType.Snippet.c_str());
                            state.ErrorRaised = true;
                            continue;
                        }

                        ShaderInputParam var{};
                        switch (fieldType.Type)
                        {
                        case ShaderTokenType::UInt:
                        case ShaderTokenType::Float: var.Size = pge::ParameterSize::Float1; break;
                        case ShaderTokenType::UInt2:
                        case ShaderTokenType::Float2: var.Size = pge::ParameterSize::Float2; break;
                        case ShaderTokenType::UInt3:
                        case ShaderTokenType::Float3: var.Size = pge::ParameterSize::Float3; break;
                        case ShaderTokenType::UInt4:
                        case ShaderTokenType::Float4: var.Size = pge::ParameterSize::Float4; break;
                        case ShaderTokenType::Byte4: var.Size = pge::ParameterSize::Byte4; break;
                        default: break;
                        }

                        ShaderToken& fieldName = state.Tokens[startToken++];
                        if (fieldName.Type != ShaderTokenType::Identifier)
                        {
                            logger->error("[{}]: Invalid name for variable decleration! Expected an identifier got: \"{}\"", fieldName.Line, fieldName.Snippet.c_str());
                            state.ErrorRaised = true;
                            continue;
                        }

                        if (state.Tokens[startToken].Type == ShaderTokenType::Colon)
                        {
                            startToken++;

                            ShaderToken& semanticId = state.Tokens[startToken++];
                            std::string_view semanticIdView = std::string_view(semanticId.Snippet.data(), semanticId.Snippet.size() - (isdigit(semanticId.Snippet.back()) > 0));
                            if (semanticId.Type != ShaderTokenType::Identifier || !std::count(s_Semantics.begin(), s_Semantics.end(), semanticIdView))
                            {
                                logger->error("[{}]: Expected semantic identifier for variable! Got: \"{}\"", semanticId.Line, semanticId.Snippet);
                                state.ErrorRaised = true;
                                continue;
                            }

                            std::string_view view = semanticId.Snippet;
                            if (view.starts_with("POSITION"))
                                var.Semantic = SemanticId::POSITION;
                            else  if (view.starts_with("SV_POSITION"))
                                var.Semantic = SemanticId::SV_POSITION;
                            else if (view.starts_with("COLOR"))
                                var.Semantic = SemanticId::COLOR;
                            else if (view.starts_with("NORMAL"))
                                var.Semantic = SemanticId::NORMAL;
                            else if (view.starts_with("TEXCOORD"))
                                var.Semantic = SemanticId::TEXCOORD;
                            else if (view.starts_with("TANGENT"))
                                var.Semantic = SemanticId::TANGENT;
                            else if (view.starts_with("BITANGENT"))
                                var.Semantic = SemanticId::BITANGENT;

                            char last = view.back();
                            if (isdigit(last))
                            {
                                auto res = std::from_chars(&last, (&last) + 1, var.Slot);
                                if (res.ec == std::errc::invalid_argument)
                                {
                                    logger->error("[{}]: Invalid semantic slot identifier: \"{}\"!", semanticId.Line, last);
                                    state.ErrorRaised = true;
                                    continue;
                                }
                            }
                        }

                        {
                            ShaderToken& check = state.Tokens[startToken++];
                            if (check.Type != ShaderTokenType::Semicolon)
                            {
                                logger->error("[{}]: Expected terminator for variable definiton! Got: {}", check.Line, check.Snippet.c_str());
                                state.ErrorRaised = true;
                                continue;
                            }
                        }

                        state.Result.Inputs.push_back(var);
                    }

                    didFind = true;
                    break;
                }
            }

            if (!didFind)
            {
                logger->error("[{}]: Failed to find struct for vertex input structure!");
                state.ErrorRaised = true;
                break;
            }

            break;
        }
        case ShaderTokenType::Texture1D:
        case ShaderTokenType::Texture2D:
        case ShaderTokenType::Texture3D:
        {
            TokenEOFCheck("Texture");

            ShaderToken& name = state.Tokens[state.Current++];
            if (name.Type != ShaderTokenType::Identifier)
            {
                logger->error("[{}]: Invalid token type after Texture decleration! Expected an identifier got: {}", token.Line, (uint8_t)name.Type);
                state.ErrorRaised = true;
                continue;
            }

            ShaderValueParam param{};
            if (state.Tokens[state.Current].Type == ShaderTokenType::Colon)
            {
                state.Current++;

                ShaderToken& visibility = state.Tokens[state.Current++];
                if (visibility.Type != ShaderTokenType::Identifier)
                {
                    logger->error("[{}]: Expected visibility identifier after StaticSamplerState decleration!", visibility.Line);
                    state.ErrorRaised = true;
                    continue;
                }

                if (visibility.Snippet == "All")
                    param.Visibility = ShaderVisibilityFlag::All;
                else if (visibility.Snippet == "VertexOnly")
                    param.Visibility = ShaderVisibilityFlag::Vertex;
                else if (visibility.Snippet == "PixelOnly")
                    param.Visibility = ShaderVisibilityFlag::Pixel;
                else
                {
                    logger->error("[{}]: Unkown shader visibility flag: \"{}\"!", visibility.Line, visibility.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }
            }

            param.Id = ValueId::Texture;
            param.Name = name.Snippet;
            param.TX.Dimension = token.Type;

            state.Result.Params.push_back(param);
            break;
        }
        case ShaderTokenType::SamplerState:
        {
            TokenEOFCheck("SamplerState");


            ShaderToken& name = state.Tokens[state.Current++];
            if (name.Type != ShaderTokenType::Identifier)
            {
                logger->error("[{}]: Invalid token type after SamplerState decleration! Expected an identifier got: {}", token.Line, (uint8_t)name.Type);
                state.ErrorRaised = true;
                continue;
            }

            ShaderValueParam param{};
            if (state.Tokens[state.Current].Type == ShaderTokenType::Colon)
            {
                state.Current++;

                ShaderToken& visibility = state.Tokens[state.Current++];
                if (visibility.Type != ShaderTokenType::Identifier)
                {
                    logger->error("[{}]: Expected visibility identifier after StaticSamplerState decleration!", visibility.Line);
                    state.ErrorRaised = true;
                    continue;
                }

                if (visibility.Snippet == "All")
                    param.Visibility = ShaderVisibilityFlag::All;
                else if (visibility.Snippet == "VertexOnly")
                    param.Visibility = ShaderVisibilityFlag::Vertex;
                else if (visibility.Snippet == "PixelOnly")
                    param.Visibility = ShaderVisibilityFlag::Pixel;
                else
                {
                    logger->error("[{}]: Unkown shader visibility flag: \"{}\"!", visibility.Line, visibility.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }
            }

            param.Id = ValueId::SamplerState;
            param.Name = name.Snippet;
            param.SP.Static = false;

            state.Result.Params.push_back(param);
            break;
        }
        case ShaderTokenType::StaticSamplerState:
        {
            TokenEOFCheck("StaticSamplerState");

            ShaderToken& name = state.Tokens[state.Current++];
            if (name.Type != ShaderTokenType::Identifier)
            {
                logger->error("[{}]: Invalid token type after StaticSamplerState decleration! Expected an identifier got: {}", token.Line, (uint8_t)name.Type);
                state.ErrorRaised = true;
                continue;
            }

            ShaderValueParam param{};
            if (state.Tokens[state.Current].Type == ShaderTokenType::Colon)
            {
                state.Current++;

                ShaderToken& visibility = state.Tokens[state.Current++];
                if (visibility.Type != ShaderTokenType::Identifier)
                {
                    logger->error("[{}]: Expected visibility identifier after StaticSamplerState decleration!", visibility.Line);
                    state.ErrorRaised = true;
                    continue;
                }

                if (visibility.Snippet == "All")
                    param.Visibility = ShaderVisibilityFlag::All;
                else if (visibility.Snippet == "VertexOnly")
                    param.Visibility = ShaderVisibilityFlag::Vertex;
                else if (visibility.Snippet == "PixelOnly")
                    param.Visibility = ShaderVisibilityFlag::Pixel;
                else
                {
                    logger->error("[{}]: Unkown shader visibility flag: \"{}\"!", visibility.Line, visibility.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }
            }

            {
                TokenEOFCheck("StaticSamplerState");

                ShaderToken& check = state.Tokens[state.Current++];
                if (check.Type != ShaderTokenType::OpenCurlyBracket)
                {
                    logger->error("[{}]: No body found after static sampler decleration!", token.Line);
                    state.ErrorRaised = true;
                    continue;
                }
            }

            param.Id = ValueId::SamplerState;
            param.Name = name.Snippet;
            param.SP.Static = true;
            
            TokenEOFCheck("StaticSamplerState");
            
            while (state.Current < state.Result.Bounds.End && state.Tokens[state.Current].Type != ShaderTokenType::CloseCurlyBracket)
            {
                ShaderToken& local = state.Tokens[state.Current++];
                if (local.Type != ShaderTokenType::Identifier)
                {
                    logger->error("[{}]: Expected identifer for StaticSamplerState field! Got: {}", token.Line, (uint8_t)local.Type);
                    state.ErrorRaised = true;
                    continue;
                }

                pge::SmallString field = local.Snippet;

                {
                    TokenEOFCheck("StaticSamplerState");

                    ShaderToken& check = state.Tokens[state.Current++];
                    if (check.Type != ShaderTokenType::Equals)
                    {
                        logger->error("[{}]: Empty StaticSamplerState field found with no value!", token.Line);
                        state.ErrorRaised = true;
                        continue;
                    }
                }

                TokenEOFCheck("StaticSamplerState");
                ShaderToken& local2 = state.Tokens[state.Current++];
                if (local2.Type != ShaderTokenType::String)
                {
                    logger->error("[{}]: StaticSamplerState field value not met with string but with: {}!", token.Line, (uint8_t)local2.Type);
                    state.ErrorRaised = true;
                    continue;
                }

                if (field == "Filter")
                {
                    param.SP.Filter = StringConvertFilter(local2.Snippet);
                    if (param.SP.Filter == pge::TextureFilter::Unkown)
                    {
                        logger->error("[{}]: Unkown or invalid filter for StaticSamplerState: \"{}\"", local2.Line, local2.Snippet.c_str());
                        state.ErrorRaised = true;
                        continue;
                    }
                }
                else if (field == "AddressU")
                {
                    param.SP.AddressU = StringConvertAddress(local2.Snippet);
                    if (param.SP.AddressU == pge::AddressMode::Unkown)
                    {
                        logger->error("[{}]: Unkown or invalid address mode for StaticSamplerState: \"{}\"", local2.Line, local2.Snippet.c_str());
                        state.ErrorRaised = true;
                        continue;
                    }
                }
                else if (field == "AddressV")
                {
                    param.SP.AddressV = StringConvertAddress(local2.Snippet);
                    if (param.SP.AddressV == pge::AddressMode::Unkown)
                    {
                        logger->error("[{}]: Unkown or invalid address mode for StaticSamplerState: \"{}\"", local2.Line, local2.Snippet.c_str());
                        state.ErrorRaised = true;
                        continue;
                    }
                }
                else
                {
                    logger->error("[{}]: Unkown StaticSamplerState field name: \"{}\"!", token.Line, field.c_str());
                    state.ErrorRaised = true;
                    continue;
                }
            }

            if (state.Result.Bounds.End < state.Current)
            {
                logger->error("[{}]: StaticStamplerState body definition does not have a defined end!", state.Tokens.end()->Line);
                state.ErrorRaised = true;
                continue;
            }

            state.Result.Params.push_back(param);

            break;
        }
        case ShaderTokenType::ConstantBuffer:
        {
            TokenEOFCheck("ConstantBuffer/StructuredBuffer");

            {
                ShaderToken& check = state.Tokens[state.Current++];
                if (check.Type != ShaderTokenType::Lessthan)
                {
                    logger->error("[{}]: Expected value type definition for ConstantBuffer/StructuredBuffer decleration! Got: \"{}\"", token.Line, check.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }
            }

            ShaderToken& valueType = state.Tokens[state.Current++];
            if (valueType.Type != ShaderTokenType::Identifier)
            {
                logger->error("[{}]: Expected identifier for ConstantBuffer/StructuredBuffer value type! Got: \"{}\"", token.Line, valueType.Snippet.c_str());
                state.ErrorRaised = true;
                continue;
            }

            {
                ShaderToken& check = state.Tokens[state.Current++];
                if (check.Type != ShaderTokenType::Greaterthan)
                {
                    logger->error("[{}]: ConstantBuffer/StructuredBuffer shader value type terminator not found! Got: \"{}\"", token.Line, check.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }
            }

            ShaderToken& name = state.Tokens[state.Current++];
            if (name.Type != ShaderTokenType::Identifier)
            {
                logger->error("[{}]: Invalid token type after ConstantBuffer/StructuredBuffer decleration! Expected an identifier got: \"{}\"", token.Line, name.Snippet.c_str());
                state.ErrorRaised = true;
                continue;
            }

            ShaderCBParam param{};

            if (state.Tokens[state.Current].Type == ShaderTokenType::Colon)
            {
                state.Current++;

                ShaderToken& visibility = state.Tokens[state.Current++];
                if (visibility.Type != ShaderTokenType::Identifier)
                {
                    logger->error("[{}]: Expected visibility identifier after ConstantBuffer/StructuredBuffer decleration!", visibility.Line);
                    state.ErrorRaised = true;
                    continue;
                }

                if (visibility.Snippet == "All")
                    param.Visibility = ShaderVisibilityFlag::All;
                else if (visibility.Snippet == "VertexOnly")
                    param.Visibility = ShaderVisibilityFlag::Vertex;
                else if (visibility.Snippet == "PixelOnly")
                    param.Visibility = ShaderVisibilityFlag::Pixel;
                else
                {
                    logger->error("[{}]: Unkown shader visibility flag: \"{}\"!", visibility.Line, visibility.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }
            }

            uint32_t startToken = UINT32_MAX;
            {
                for (auto& pair : structIndices)
                {
                    if (pair.first == valueType.Snippet.c_str())
                    {
                        startToken = pair.second;
                        break;
                    }
                }

                if (startToken == UINT32_MAX)
                {
                    logger->error("[{}]: Failed to find struct for ConstantBuffer/StructuredBuffer with name: \"{}\"", token.Line, valueType.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }
            }

            {
                ShaderToken& check = state.Tokens[startToken++];
                if (check.Type != ShaderTokenType::OpenCurlyBracket)
                {
                    logger->error("[{}]: No body definiton for ConstantBuffer/StructuredBuffer struct found!", token.Line);
                    state.ErrorRaised = true;
                    continue;
                }
            }

            param.Name = name.Snippet;
            param.IsStructured = token.Type == ShaderTokenType::StructuredBuffer;
            param.ValueType = valueType.Snippet;

            while (startToken < state.Tokens.size() && state.Tokens[startToken].Type != ShaderTokenType::CloseCurlyBracket)
            {
                ShaderToken& fieldType = state.Tokens[startToken++];
                if (!std::count(s_ValueKeywords.begin(), s_ValueKeywords.end(), fieldType.Type))
                {
                    logger->error("[{}]: Expected generic value type for variable decleration! Got: \"{}\"", token.Line, fieldType.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }

                ShaderToken& fieldName = state.Tokens[startToken++];
                if (fieldName.Type != ShaderTokenType::Identifier)
                {
                    logger->error("[{}]: Invalid name for variable decleration! Expected an identifier got: \"{}\"", token.Line, fieldName.Snippet.c_str());
                    state.ErrorRaised = true;
                    continue;
                }

                ShaderVariableData var{};

                if (state.Tokens[startToken].Type == ShaderTokenType::Colon)
                {
                    startToken++;

                    ShaderToken& semanticId = state.Tokens[startToken++];
                    if (semanticId.Type != ShaderTokenType::Identifier || !std::count(s_Semantics.begin(), s_Semantics.end(), (std::string_view)semanticId.Snippet))
                    {
                        logger->error("[{}]: Expected semantic identifier for variable! Got: \"{}\"", token.Line, (uint8_t)semanticId.Type);
                        state.ErrorRaised = true;
                        continue;
                    }

                    std::string_view view = semanticId.Snippet;
                    if (view.starts_with("POSITION"))
                        var.Semantic = SemanticId::POSITION;
                    else  if (view.starts_with("SV_POSITION"))
                        var.Semantic = SemanticId::SV_POSITION;
                    else if (view.starts_with("COLOR"))
                        var.Semantic = SemanticId::COLOR;
                    else if (view.starts_with("NORMAL"))
                        var.Semantic = SemanticId::NORMAL;
                    else if (view.starts_with("TEXCOORD"))
                        var.Semantic = SemanticId::TEXCOORD;
                    else if (view.starts_with("TANGENT"))
                        var.Semantic = SemanticId::TANGENT;
                    else if (view.starts_with("BITANGENT"))
                        var.Semantic = SemanticId::BITANGENT;

                    char last = view[view.size() - 1];
                    if (isdigit(last))
                    {
                        auto res = std::from_chars(&last, &last, var.Slot);
                        if (res.ec == std::errc::invalid_argument)
                        {
                            logger->error("[{}]: Invalid semantic slot identifier: \"{}\"!", semanticId.Line, last);
                            state.ErrorRaised = true;
                            continue;
                        }
                    }
                }

                {
                    ShaderToken& check = state.Tokens[startToken++];
                    if (check.Type != ShaderTokenType::Semicolon)
                    {
                        logger->error("[{}]: Expected terminator for variable definiton! Got: {}", token.Line, check.Snippet.c_str());
                        state.ErrorRaised = true;
                        continue;
                    }
                }

                var.Name = fieldName.Snippet;
                var.VariableType = fieldType.Type;
                param.Variables.push_back(var);
            }

            state.Result.CBParams.push_back(param);

            break;
        }
        default:
            //Generally these are redundant and break it. Maybe there are some special situations but i dont know them at this point.
            //logger->error("[{}]: Unkown or invalid token encountered: {} at: {}/{}", token.Line, (uint8_t)token.Type, state.Current + 1, state.Tokens.size());
            //state.ErrorRaised = true;
            break;
        }
    }

    return !state.ErrorRaised;
}

bool ed::ShaderParser::PerformAST(LocalState& state)
{
    auto logger = spdlog::get("ED");

    state.Start = 0;
    state.Current = 0;

    uint8_t bodyDepth = 0;

    while (state.Current < state.Tokens.size())
    {
        state.Start = state.Current;
        ShaderToken& token = state.Tokens[state.Current++];

        switch (token.Type)
        {
        case ShaderTokenType::Struct:
        {
            if (bodyDepth > 0)
            {
                logger->error("[{}]: Struct can only be defined outside of scope!", token.Line);
                state.ErrorRaised = true;
                continue;
            }

            bodyDepth++;
            ASTStruct(state);
            bodyDepth--;

            break;
        }
        case ShaderTokenType::ConstantBuffer: ASTConstantBuffer(state); break;
        case ShaderTokenType::SamplerState: ASTSamplerState(state); break;
        case ShaderTokenType::StaticSamplerState: ASTStaticSamplerState(state); break;
        case ShaderTokenType::Texture1D:
        case ShaderTokenType::Texture1DArray:
        case ShaderTokenType::Texture2D:
        case ShaderTokenType::Texture2DArray:
        case ShaderTokenType::Texture3D:
        case ShaderTokenType::Texture3DArray: ASTTexture(state, token); break;
        case ShaderTokenType::Semicolon: break;
        case ShaderTokenType::Identifier:
        {
            if (bodyDepth > 0)
            {
                ShaderToken& post = state.Tokens[state.Current + 1];
                if (ASTIsFunction(state, post.Snippet))
                    ASTCall(state, token);
                else
                    ASTVariable(state, token);
            }
            else
            {
                bool errBefore = state.ErrorRaised;
                ASTFunction(state, token); //must be a function! HLSL doesnt support global scope variables (only special ones that are handled above!)

                //avoid misinformation
                if (!errBefore && !state.ErrorRaised) bodyDepth++;

                //skip potential function! may skip more then expected but.. eh: it is what it is
                if (state.ErrorRaised && state.Tokens[state.Current - 1].Type == ShaderTokenType::OpenCurlyBracket)
                    while (state.Tokens[state.Current].Type != ShaderTokenType::CloseCurlyBracket) state.Current++;
            }

            break;
        }
        case ShaderTokenType::CloseCurlyBracket:
        {
            if (bodyDepth <= 0)
            {
                logger->error("[{}]: Cannot end body definition with a depth of 0!", token.Line);
                state.ErrorRaised = true;
                continue;
            }

            bodyDepth--;
            state.AST_Active = nullptr;

            break;
        }
        default:
            logger->error("[{}]: Unkown or invalid token encountered: \"{}\"!", token.Line, token.Snippet.c_str());
            state.ErrorRaised = true;
            break;
        }
    }

    return !state.ErrorRaised;
}

bool ed::ShaderParser::PerformBuild(LocalState& state)
{
    auto logger = spdlog::get("ED");

    std::vector<ShaderFunctionDefinition> functions;
    if (!ExamineAllFunctions(state, functions))
    {
        logger->error("Failed to examine functions for rebuiling source!");
        state.ErrorRaised = true;
        return false;
    }

    state.Start = 0;
    state.Current = state.Result.Bounds.Start;

    bool enteringFunction = false;
    ShaderVisibilityFlag enteringType = ShaderVisibilityFlag::All;
    
    uint8_t skippingType = 0;

    std::stringstream builder{};

    //pregenerate "RESOURCES" cb
    {
        builder << "struct GLOBAL_RESOURCES\n{\n";

        uint16_t indexGlobal = 0;
        for (auto& cb : state.Result.CBParams)
        {
            cb.Index = indexGlobal++;

            builder << "    uint " << cb.Name.c_str() << "_INDEX;\n";
        }

        for (auto& val : state.Result.Params)
        {
            val.Index = indexGlobal++;

            builder << "    uint " << val.Name.c_str() << "_INDEX;\n";
        }

        builder << "};\n\nConstantBuffer<GLOBAL_RESOURCES> RESOURCES : register(b0);\n\n";
    }

    while (state.Current < state.Result.Bounds.End)
    {
        state.Start = state.Current;
        ShaderToken& token = state.Tokens[state.Current++];

        if (skippingType > 0)
        {
            switch (skippingType)
            {
            case 1: if (token.Type == ShaderTokenType::Semicolon) skippingType = 0; break;
            case 2: if (token.Type == ShaderTokenType::CloseCurlyBracket) skippingType = 0; break;
            default: skippingType = 0; break;
            }

            continue;
        }

        switch (token.Type)
        {
        case ShaderTokenType::Struct: builder << "struct "; break;
        case ShaderTokenType::Return: builder << "return "; break;

        case ShaderTokenType::Float:
        case ShaderTokenType::Float2:
        case ShaderTokenType::Float3:
        case ShaderTokenType::Float4:
        case ShaderTokenType::Float4x4:
        case ShaderTokenType::UInt:
        case ShaderTokenType::UInt2:
        case ShaderTokenType::UInt3:
        case ShaderTokenType::UInt4:
        case ShaderTokenType::Byte4:
        case ShaderTokenType::Identifier: 
        {
            auto find = std::find_if(functions.begin(), functions.end(), [&](ShaderFunctionDefinition& compare) { return compare.LocalStart == state.Current; });
            if (find != functions.end() && !enteringFunction)
            {
                enteringFunction = true;
                enteringType = (state.Tokens[state.Current].Snippet == "pixel") ? ShaderVisibilityFlag::Pixel : ShaderVisibilityFlag::Vertex;
            }

            if (token.Snippet == "byte4")
                builder << "float4 ";
            else
                builder << token.Snippet.c_str() << " ";
            break;
        }
        case ShaderTokenType::Number: builder << fmt::format("{:f}", token.FloatValue) << "f "; break;
        case ShaderTokenType::Integral: builder << token.UIntValue; break;

        case ShaderTokenType::Colon: builder.write(":", 1); break;
        case ShaderTokenType::Semicolon: builder.write(";\n", 2); break;
        case ShaderTokenType::Period: builder.write(",", 1); break;
        case ShaderTokenType::Comma: builder.write(".", 1); break;
        case ShaderTokenType::Underscore: builder.write("_", 1); break;
        case ShaderTokenType::Lessthan: builder.write("<", 1); break;
        case ShaderTokenType::Greaterthan: builder.write(">", 1); break;
        case ShaderTokenType::Plus: builder.write("+", 1); break;
        case ShaderTokenType::Minus: builder.write("-", 1); break;
        case ShaderTokenType::Star: builder.write("*", 1); break;
        case ShaderTokenType::Slash: builder.write("/", 1); break;
        case ShaderTokenType::Equals: builder.write("=", 1); break;

        case ShaderTokenType::OpenSquareBracket: builder.write("[", 1); break;
        case ShaderTokenType::CloseSquareBracket: builder.write("]", 1); break;
        case ShaderTokenType::OpenBracket: builder.write("(", 1); break;
        case ShaderTokenType::CloseBracket: builder.write(")", 1); break;
        case ShaderTokenType::OpenCurlyBracket:
        {
            builder.write("{\n", 2);
            if (enteringFunction)
            {
                for (ShaderCBParam& cb : state.Result.CBParams)
                {
                    if (cb.Visibility == ShaderVisibilityFlag::All || cb.Visibility == enteringType)
                    {
                        builder << (cb.IsStructured ? "StructuredBuffer<" : "ConstantBuffer<") << cb.ValueType.c_str() << "> " << cb.Name.c_str() << " = ResourceDescriptorHeap[RESOURCES." << cb.Name.c_str() << "_INDEX];\n";
                    }
                }

                for (ShaderValueParam& val : state.Result.Params)
                {
                    if (val.Visibility == ShaderVisibilityFlag::All || val.Visibility == enteringType)
                    {
                        switch (val.Id)
                        {
                        case ValueId::Texture:
                        {
                            std::string_view dim = "";
                            switch (val.TX.Dimension)
                            {
                            case ShaderTokenType::Texture1D: dim = "Texture1D"; break;
                            case ShaderTokenType::Texture1DArray: dim = "Texture1DArray"; break;
                            case ShaderTokenType::Texture2D: dim = "Texture2D"; break;
                            case ShaderTokenType::Texture2DArray: dim = "Texture2DArray"; break;
                            case ShaderTokenType::Texture3D: dim = "Texture3D"; break;
                            case ShaderTokenType::Texture3DArray: dim = "Texture3DArray"; break;
                            default: break;
                            }

                            builder << dim << " " << val.Name.c_str() << " = ResourceDescriptorHeap[RESOURCES." << val.Name.c_str() << "_INDEX];\n";
                            break;
                        }
                        case ValueId::SamplerState:
                        {
                            if (val.SP.Static) continue;
                            builder << "SamplerState " << val.Name.c_str() << " = ResourceDescriptorHeap[RESOURCES." << val.Name.c_str() << "_INDEX];\n";
                            break;
                        }
                        default: break;
                        }
                    }
                }

                enteringFunction = false;
                enteringType = ShaderVisibilityFlag::All;
            }

            break;
        }
        case ShaderTokenType::CloseCurlyBracket: builder.write("\n}\n", 3); break;

        case ShaderTokenType::ConstantBuffer: skippingType = 1; break;
        case ShaderTokenType::SamplerState: skippingType = 1; break;
        case ShaderTokenType::StaticSamplerState: builder << "SamplerState " << state.Tokens[state.Current].Snippet.c_str() << ";\n"; skippingType = 2; break;
        case ShaderTokenType::Texture1D: skippingType = 1; break;
        case ShaderTokenType::Texture1DArray: skippingType = 1; break;
        case ShaderTokenType::Texture2D: skippingType = 1; break;
        case ShaderTokenType::Texture2DArray: skippingType = 1; break;
        case ShaderTokenType::Texture3D: skippingType = 1; break;
        case ShaderTokenType::Texture3DArray: skippingType = 1; break;

        default:
            logger->error("[{}]: Unkown or invalid token encountered: \"{}\"", token.Line, token.Snippet.c_str());
            state.ErrorRaised = true;
            break;
        }
    }

    if (!state.ErrorRaised)
    {
        size_t size = builder.tellp();
        state.Result.ResultBuffer.resize(size);
        
        builder.seekp(0);
        builder.read(state.Result.ResultBuffer.data(), size);
    }

    return !state.ErrorRaised;
}

bool ed::ShaderParser::ParseShader(std::vector<char>& source, ShaderParseResult& result)
{
    auto logger = spdlog::get("ED");

    LocalState state{ .Result = result, .Source = source };

    logger->info("Performing scan on source..");
    if (!PerformScan(state))
    {
        logger->error("Failed to scan shader source file!");
        return false;
    }

    logger->info("Performing bounds on source..");
    if (!PerformBounds(state))
    {
        logger->error("Failed to bound shader source file!");
        return false;
    }

    logger->info("Performing reflect on source..");
    if (!PerformReflect(state))
    {
        logger->error("Failed to reflect shader source file!");
        return false;
    }

    /*logger->info("Performing AST on source..");
    if (!PerformAST(state))
    {
        logger->error("Failed to AST shader source file!");
        return false;
    }*/

    logger->info("Performing build on source..");
    if (!PerformBuild(state))
    {
        logger->error("Failed to build shader source file!");
        return false;
    }

    return true;
}

bool ed::ShaderParser::ParseTags(std::vector<char>& source, ShaderTags& tags)
{
    ShaderParseResult result{};
    LocalState state{ .Result = result,.Source = source };

    if (!PerformScan(state)) return false;
    if (!PerformBounds(state)) return false;

    tags.Values = result.Tags.Values;
    return true;
}
