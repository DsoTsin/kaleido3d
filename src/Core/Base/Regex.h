#pragma once
// forward decl
struct pcre2_real_code_8;
struct pcre2_real_match_data_8;
// end forward decl
namespace k3d
{
    class RegExPrivate;

    class K3D_CORE_API RegEx
    {
    public:
        enum Option
        {
            Default = 0,
            IgnoreCase = 1,
            MultiLine = 2,
        };

        struct GroupElement
        {
            String Name;
            int Index = 0;
            int Start = 0;
            int Length = 0;
        };
        
        class K3D_CORE_API Group
        {
        public:
            enum ESubGroup
            {
                Named,
                NonNamed
            };
            Group(int NumMatches, const char* Ptr)
                : m_GroupTypes(NumMatches, NonNamed)
                , m_Ptr(Ptr)
            {
            }

            String SubGroup(int Id) const;
            String SubGroup(const char* Name) const;

            Group& SetType(int Index, ESubGroup Type)
            {
                m_GroupTypes[Index] = Type;
                return *this;
            }
            bool IsNamed(int i) const
            {
                return m_GroupTypes[i] == Named;
            }
            void AppendNamedElement(GroupElement&& Element)
            {
                m_SubNamedGroups.Append(Element);
            }

            void AppendNonNamedElement(GroupElement&& Element)
            {
                m_SubNonNamedGroups.Append(Element);
            }

        private:
            friend class RegEx;
            const char*         m_Ptr;
            DynArray<ESubGroup> m_GroupTypes;
            DynArray<GroupElement> m_SubNamedGroups;
            DynArray<GroupElement> m_SubNonNamedGroups;
        };

        using Groups = DynArray<Group>;

        RegEx(const char* Expr, Option Mode);
        ~RegEx();

        bool IsValid() const;

        String GetError() const;

        bool Match(const char* Str, Option InOption = Default);
        bool Match(const char* Str, Groups& OutGroups, Option InOption = Default);

    private:
        pcre2_real_code_8*  m_Code;
        Option              m_Mode;
        int                 m_CompileErrorNo;
        size_t              m_CompileErrorOffset;
    };
}