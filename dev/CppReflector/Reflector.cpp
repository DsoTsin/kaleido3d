#include <clang-c/Index.h>
#include <Core/Kaleido3D.h>
#include <Core/KTL/String.hpp>
#include <Core/KTL/DynArray.hpp>
#include <Core/LogUtil.h>
#include "Reflector.h"

namespace k3d
{

    String cxStr(CXString str)
    {
        String _Str(clang_getCString(str));
        clang_disposeString(str);
        return _Str;
    }

    String GetFullName(CXCursor cursor)
    {
        String name;
        while (clang_isDeclaration(clang_getCursorKind(cursor)) != 0)
        {
            String cur = cxStr(clang_getCursorSpelling(cursor));
            if (name.Length() == 0)
            {
                name = cur;
            }
            else
            {
                name = cur + String("::") + name;
            }
            cursor = clang_getCursorSemanticParent(cursor);
        }

        return name;
    }

    Cursor::Cursor(const CXCursor & c, CursorType _Type)
        : m_Handle(c)
        , Type(_Type)
    {
    }

    CXChildVisitResult VisitAnnotation(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        CXCursorKind Kind = clang_getCursorKind(cursor);
        switch (Kind)
        {
        case CXCursor_AnnotateAttr:
            KLOG(Info, annotate, "===> %s.", cxStr(clang_getCursorSpelling(cursor)).CStr());
            return CXChildVisit_Break;
        }
        return CXChildVisit_Recurse;
    }

    String Cursor::GetName() const
    {
        return cxStr(clang_getCursorDisplayName(m_Handle));
    }

    String Cursor::GetFullTypeName()
    {
        return GetFullName(m_Handle);
    }

    Cursor::List Cursor::GetChildren()
    {
        Cursor::List list;

        auto visitor = [](CXCursor cursor, CXCursor parent, CXClientData data)
        {
            auto name = GetFullName(cursor);
            auto kind = clang_getCursorKind(cursor);
            Cursor::List* _List = reinterpret_cast<Cursor::List*>(data);
            switch (kind)
            {
            case CXCursor_EnumDecl:
                KLOG(Info, Enum, "EnumName: %s.", name.CStr());
                clang_visitChildren(cursor, VisitAnnotation, nullptr);
                break;
            case CXCursor_ClassDecl:
            case CXCursor_TypedefDecl:
            case CXCursor_StructDecl:
            {
                String annotate = cxStr(clang_getCursorUSR(cursor));
                if (annotate.Length() != 0)
                {
                    KLOG(Info, Class, "ClassName: %s. Attonation: (%s)", name.CStr(), annotate.CStr());
                }
                else
                {
                    KLOG(Info, Class, "ClassName: %s.", name.CStr());
                }
                _List->Append(Cursor(cursor, CursorType::Class));
                clang_visitChildren(cursor, VisitAnnotation, nullptr);
                break;
            }
            //case CXCursor_CXXMethod:
            case CXCursor_FunctionDecl:
                KLOG(Info, Function, "FunctionName: %s.", name.CStr());
                _List->Append(Cursor(cursor, CursorType::Function));
                clang_visitChildren(cursor, VisitAnnotation, nullptr);
                break;
            case CXCursor_FieldDecl:
                KLOG(Info, Field, "FieldName: %s.", name.CStr());
                clang_visitChildren(cursor, VisitAnnotation, nullptr);
                break;
            case CXCursor_MacroDefinition:
                KLOG(Info, Define, "DefineName: %s.", name.CStr());
                _List->Append(Cursor(cursor, CursorType::MacroDefinition));
                break;
            default:
                break;
            }
            return CXChildVisit_Recurse;
        };

        clang_visitChildren(m_Handle, visitor, &list);
        return list;
    }

    Parser::Parser(const char* FileName, const char* _argv[], int argc)
    {
        ParIndex = clang_createIndex(0, 0);
        ParUnit = clang_parseTranslationUnit(
            ParIndex,
            FileName, _argv, argc - 3,
            nullptr, 0,
            CXTranslationUnit_None);
    }

    Parser::~Parser()
    {
        clang_disposeTranslationUnit(ParUnit);
        clang_disposeIndex(ParIndex);
    }

    void Parser::DoParse()
    {
        if (ParUnit == nullptr)
        {
            KLOG(Fatal, CppReflector, "Unable to parse translation unit. Quitting.");
            return;
        }
        k3d::Cursor cur(clang_getTranslationUnitCursor(ParUnit));
        DumpFile.Open("Dump.txt", IOWrite);
        for (auto Child : cur.GetChildren())
        {
            auto CurName = Child.GetFullTypeName();
            String Data;
            switch (Child.GetType())
            {
            case CursorType::Class:
                Data += "C ";
                break;

            case CursorType::Function:
                Data += "F ";
                break;
            case CursorType::MacroDefinition:
                Data += "D ";
                break;
            } 
            Data += CurName + "\r\n";
            DumpFile.Write(*Data, Data.Length());
        }
        DumpFile.Close();
    }
}