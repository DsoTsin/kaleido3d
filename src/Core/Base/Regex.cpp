#include "CoreMinimal.h"

#define PCRE2_STATIC 1
#define PCRE2_CODE_UNIT_WIDTH 8

#if 0
#include <pcre2.h>

namespace k3d
{
    class MatchBase
    {
    public:
        MatchBase(pcre2_real_code_8* InRE)
            : m_RE(InRE)
        {
            m_Data = pcre2_match_data_create_from_pattern(m_RE, NULL);
        }
        virtual ~MatchBase()
        {
            if (m_Data)
            {
                pcre2_match_data_free(m_Data);
                m_Data = nullptr;
            }
        }
        bool Search(const char* Str)
        {
            int rc = pcre2_match(m_RE, (PCRE2_SPTR)Str, strlen(Str),
                0,                      /* starting offset in the subject */
                0,                      /* options */
                m_Data,
                NULL);                  /* use default match context */
            if (rc < 0)
            {
                return false;
            }
            return rc > 0;
        }
    protected:
        pcre2_real_code_8*          m_RE;
        pcre2_real_match_data_8*    m_Data;
    };

    class MatchGroup : public MatchBase
    {
    public:
        MatchGroup(pcre2_real_code_8* InRe, RegEx::Groups& OutGroup);
        ~MatchGroup();

        bool Match(const char* Str);
        bool MatchAll(const char* Str);

    private:
        RegEx::Groups&              m_Groups;
    };

    RegEx::RegEx(const char * Expr, Option Mode)
        : m_Code(nullptr)
        , m_Mode(Mode)
        , m_CompileErrorNo(0)
        , m_CompileErrorOffset(0)
    {
        /*
        if (opt & RegEx::IgnoreCase)
        {
            m_Flags |= PCRE2_CASELESS;
        }*/
        m_Code = pcre2_compile(
            (PCRE2_SPTR)Expr, PCRE2_ZERO_TERMINATED,
            PCRE2_UTF,
            &m_CompileErrorNo, &m_CompileErrorOffset,
            NULL);
    }

    RegEx::~RegEx()
    {
        if (m_Code)
        {
            pcre2_code_free(m_Code);
            m_Code = nullptr;
        }
    }

    bool RegEx::IsValid() const
    {
        return m_Code != nullptr;
    }

    String RegEx::GetError() const
    {
        PCRE2_UCHAR buffer[256] = { 0 };
        pcre2_get_error_message(m_CompileErrorNo, buffer, sizeof(buffer));
        return String::Format("PCRE2 compilation failed at offset %d: %s\n", (int)m_CompileErrorOffset, buffer);
    }

    bool RegEx::Match(const char * Str, Option option)
    {
        MatchBase match(m_Code);
        return match.Search(Str);
    }

    bool RegEx::Match(const char * Str, Groups& OutGroups, Option InOption)
    {
        MatchGroup match(m_Code, OutGroups);
        if (InOption & Option::MultiLine)
        {
            return match.MatchAll(Str);
        }
        return match.Match(Str);
    }

    String RegEx::Group::SubGroup(int Id) const
    {
        auto Elem = m_SubNonNamedGroups[Id];
        return String(m_Ptr+Elem.Start, Elem.Length);
    }

    String RegEx::Group::SubGroup(const char * Name) const
    {
        int i = 0;
        for (; i < m_SubNamedGroups.Count(); i++)
        {
            if (!strcmp(m_SubNamedGroups[i].Name.Data(),Name))
                break;
        }
        return String(m_Ptr + m_SubNamedGroups[i].Start, m_SubNamedGroups[i].Length);
    }

    MatchGroup::MatchGroup(pcre2_real_code_8 * InRe, RegEx::Groups& OutGroup)
        : MatchBase(InRe)
        , m_Groups(OutGroup)
    {
    }

    MatchGroup::~MatchGroup()
    {
    }

    bool MatchGroup::Match(const char * Str)
    {
        int rc = pcre2_match(m_RE, (PCRE2_SPTR)Str, strlen(Str),
            0,                  /* starting offset in the subject */
            PCRE2_ANCHORED,                  /* options */
            m_Data,             /* block for storing the result */
            NULL);              /* use default match context */

        if (rc <= 0)
            return false;

        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(m_Data);
        int namecount = 0;
        pcre2_pattern_info(m_RE, PCRE2_INFO_NAMECOUNT, &namecount);
        
        RegEx::Group group(rc, Str);
        if (namecount > 0)
        {
            int name_entry_size = 0;
            PCRE2_SPTR tabptr;
            pcre2_pattern_info(m_RE, PCRE2_INFO_NAMETABLE, &tabptr);
            pcre2_pattern_info(m_RE, PCRE2_INFO_NAMEENTRYSIZE, &name_entry_size);
            for (int i = 0; i < namecount; i++)
            {
                int index = (tabptr[0] << 8) | tabptr[1];
                int name_len = name_entry_size - 3;
                PCRE2_SPTR name_ptr = tabptr + 2;
                int value_len = (int)(ovector[2 * index + 1] - ovector[2 * index]);
                PCRE2_SPTR value_ptr = (PCRE2_SPTR)Str + ovector[2 * index];
                tabptr += name_entry_size;
                RegEx::GroupElement element;
                element.Name = Move(String(name_ptr, name_len));
                element.Index = index;
                element.Start = ovector[2 * index];
                element.Length = value_len;
                group.SetType(index, RegEx::Group::Named);
                group.AppendNamedElement(Move(element));
            }
        }

        for (int i = 1; i < rc; i++)
        {
            if (!group.IsNamed(i))
            {
                RegEx::GroupElement element;
                element.Index = i;
                element.Start = ovector[2 * i];
                element.Length = ovector[2 * i + 1] - ovector[2 * i];
                group.AppendNonNamedElement(Move(element));
            }
        }
        m_Groups.Append(group);
        return true;
    }

    bool MatchGroup::MatchAll(const char * Str)
    {
        size_t strLen = strlen(Str);
        int rc = pcre2_match(m_RE, (PCRE2_SPTR)Str, strLen,
            0,                  /* starting offset in the subject */
            PCRE2_ANCHORED,                  /* options */
            m_Data,             /* block for storing the result */
            NULL);              /* use default match context */

        if (rc <= 0)
            return false;

        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(m_Data);
        int namecount = 0;
        pcre2_pattern_info(m_RE, PCRE2_INFO_NAMECOUNT, &namecount);

        RegEx::Group group(rc, Str);
        int name_entry_size = 0;
        PCRE2_SPTR tabptr;
        if (namecount > 0)
        {
            pcre2_pattern_info(m_RE, PCRE2_INFO_NAMETABLE, &tabptr);
            pcre2_pattern_info(m_RE, PCRE2_INFO_NAMEENTRYSIZE, &name_entry_size);
            for (int i = 0; i < namecount; i++)
            {
                int index = (tabptr[0] << 8) | tabptr[1];
                int name_len = name_entry_size - 3;
                PCRE2_SPTR name_ptr = tabptr + 2;
                int value_len = (int)(ovector[2 * index + 1] - ovector[2 * index]);
                PCRE2_SPTR value_ptr = (PCRE2_SPTR)Str + ovector[2 * index];
                tabptr += name_entry_size;
                RegEx::GroupElement element;
                element.Name = Move(String(name_ptr, name_len));
                element.Index = index;
                element.Start = ovector[2 * index];
                element.Length = value_len;
                group.SetType(index, RegEx::Group::Named);
                group.AppendNamedElement(Move(element));
            }
        }

        for (int i = 1; i < rc; i++)
        {
            if (!group.IsNamed(i))
            {
                RegEx::GroupElement element;
                element.Index = i;
                element.Start = ovector[2 * i];
                element.Length = ovector[2 * i + 1] - ovector[2 * i];
                group.AppendNonNamedElement(Move(element));
            }
        }
        m_Groups.Append(group);

        U32 option_bits;
        (void)pcre2_pattern_info(m_RE, PCRE2_INFO_ALLOPTIONS, &option_bits);
        bool utf8 = (option_bits & PCRE2_UTF) != 0;

        /* Now find the newline convention and see whether CRLF is a valid newline
        sequence. */
        U32 newline;
        (void)pcre2_pattern_info(m_RE, PCRE2_INFO_NEWLINE, &newline);
        bool crlf_is_newline = newline == PCRE2_NEWLINE_ANY ||
            newline == PCRE2_NEWLINE_CRLF ||
            newline == PCRE2_NEWLINE_ANYCRLF;

        for (;;)
        {
            uint32_t options = 0;                   /* Normally no options */
            PCRE2_SIZE start_offset = ovector[1];   /* Start at end of previous match */

                                                    /* If the previous match was for an empty string, we are finished if we are
                                                    at the end of the subject. Otherwise, arrange to run another match at the
                                                    same point to see if a non-empty match can be found. */

            if (ovector[0] == ovector[1])
            {
                if (ovector[0] == strLen) break;
                options = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
            }

            /* If the previous match was not an empty string, there is one tricky case to
            consider. If a pattern contains \K within a lookbehind assertion at the
            start, the end of the matched string can be at the offset where the match
            started. Without special action, this leads to a loop that keeps on matching
            the same substring. We must detect this case and arrange to move the start on
            by one character. The pcre2_get_startchar() function returns the starting
            offset that was passed to pcre2_match(). */

            else
            {
                PCRE2_SIZE startchar = pcre2_get_startchar(m_Data);
                if (start_offset <= startchar)
                {
                    if (startchar >= strLen) break;   /* Reached end of subject.   */
                    start_offset = startchar + 1;             /* Advance by one character. */
                    if (utf8)                                 /* If UTF-8, it may be more  */
                    {                                       /*   than one code unit.     */
                        for (; start_offset < strLen; start_offset++)
                            if ((Str[start_offset] & 0xc0) != 0x80) break;
                    }
                }
            }

            /* Run the next matching operation */

            rc = pcre2_match(
                m_RE,                   /* the compiled pattern */
                (PCRE2_SPTR)Str,              /* the subject string */
                strLen,       /* the length of the subject */
                start_offset,         /* starting offset in the subject */
                options,              /* options */
                m_Data,           /* block for storing the result */
                NULL);                /* use default match context */

                                      /* This time, a result of NOMATCH isn't an error. If the value in "options"
                                      is zero, it just means we have found all possible matches, so the loop ends.
                                      Otherwise, it means we have failed to find a non-empty-string match at a
                                      point where there was a previous empty-string match. In this case, we do what
                                      Perl does: advance the matching position by one character, and continue. We
                                      do this by setting the "end of previous match" offset, because that is picked
                                      up at the top of the loop as the point at which to start again.
                                      There are two complications: (a) When CRLF is a valid newline sequence, and
                                      the current position is just before it, advance by an extra byte. (b)
                                      Otherwise we must ensure that we skip an entire UTF character if we are in
                                      UTF mode. */

            if (rc == PCRE2_ERROR_NOMATCH)
            {
                if (options == 0) break;                    /* All matches found */
                ovector[1] = start_offset + 1;              /* Advance one code unit */
                if (crlf_is_newline &&                      /* If CRLF is a newline & */
                    start_offset < strLen - 1 &&    /* we are at CRLF, */
                    Str[start_offset] == '\r' &&
                    Str[start_offset + 1] == '\n')
                    ovector[1] += 1;                          /* Advance by one more. */
                else if (utf8)                              /* Otherwise, ensure we */
                {                                         /* advance a whole UTF-8 */
                    while (ovector[1] < strLen)       /* character. */
                    {
                        if ((Str[ovector[1]] & 0xc0) != 0x80) break;
                        ovector[1] += 1;
                    }
                }
                continue;    /* Go round the loop again */
            }

            /* Other matching errors are not recoverable. */

            if (rc < 0)
            {
                printf("Matching error %d\n", rc);
                pcre2_match_data_free(m_Data);
                pcre2_code_free(m_RE);
                return 1;
            }

            /* Match succeded */

            printf("\nMatch succeeded again at offset %d\n", (int)ovector[0]);

            /* The match succeeded, but the output vector wasn't big enough. This
            should not happen. */

            if (rc == 0)
                printf("ovector was not big enough for all the captured substrings\n");

            /* We must guard against patterns such as /(?=.\K)/ that use \K in an
            assertion to set the start of a match later than its end. In this
            demonstration program, we just detect this case and give up. */

            if (ovector[0] > ovector[1])
            {
                printf("\\K was used in an assertion to set the match start after its end.\n"
                    "From end to start the match was: %.*s\n", (int)(ovector[0] - ovector[1]),
                    (char *)(Str + ovector[1]));
                printf("Run abandoned\n");
                pcre2_match_data_free(m_Data);
                pcre2_code_free(m_RE);
                return 1;
            }

            /* As before, show substrings stored in the output vector by number, and then
            also any named substrings. */

            for (int i = 0; i < rc; i++)
            {
                PCRE2_SPTR substring_start = (PCRE2_SPTR)Str + ovector[2 * i];
                size_t substring_length = ovector[2 * i + 1] - ovector[2 * i];
                printf("%2d: %.*s\n", i, (int)substring_length, (char *)substring_start);
            }

            if (namecount == 0) printf("No named substrings\n"); else
            {
                printf("Named substrings\n");
                for (int i = 0; i < namecount; i++)
                {
                    int n = (tabptr[0] << 8) | tabptr[1];
                    printf("(%d) %*s: %.*s\n", n, name_entry_size - 3, tabptr + 2,
                        (int)(ovector[2 * n + 1] - ovector[2 * n]), Str + ovector[2 * n]);
                    tabptr += name_entry_size;
                }
            }
        }
        return true;
    }

}
#endif