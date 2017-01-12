#include "Kaleido3D.h"
#include "String.h"
#include <cstdarg>
#include <string.h>

K3D_COMMON_NS
{
    int Vsnprintf(char*dest, int n, const char* fmt, va_list list)
    {
        return ::vsnprintf(dest, n, fmt, list);
    }
}
