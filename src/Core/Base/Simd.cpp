#include "CoreMinimal.h"
#include "Simd.h"

#if K3D_USE_SSE
#include <smmintrin.h>
#endif

namespace k3d
{
    namespace simd
    {
        void MemoryCopy(void* __restrict Dst, const void* __restrict Src, size_t NumQuadwords)
        {
#if K3D_USE_SSE

//            assert(kMath::IsAligned(Dst, 16));
//            assert(kMath::IsAligned(Src, 16));

            __m128i* __restrict Dest = (__m128i* __restrict)Dst;
            const __m128i* __restrict Source = (const __m128i* __restrict)Src;

            // Discover how many quadwords precede a cache line boundary.  Copy them separately.
            size_t InitialQuadwordCount = (4 - ((size_t)Source >> 4) & 3) & 3;
            if (InitialQuadwordCount > NumQuadwords)
                InitialQuadwordCount = NumQuadwords;

            switch (InitialQuadwordCount)
            {
            case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));	 // Fall through
            case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));	 // Fall through
            case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));	 // Fall through
            default:
                break;
            }

            if (NumQuadwords == InitialQuadwordCount)
                return;

            Dest += InitialQuadwordCount;
            Source += InitialQuadwordCount;
            NumQuadwords -= InitialQuadwordCount;

            size_t CacheLines = NumQuadwords >> 2;

            switch (CacheLines)
            {
            default:
            case 10: _mm_prefetch((char*)(Source + 36), _MM_HINT_NTA);	// Fall through
            case 9:  _mm_prefetch((char*)(Source + 32), _MM_HINT_NTA);	// Fall through
            case 8:  _mm_prefetch((char*)(Source + 28), _MM_HINT_NTA);	// Fall through
            case 7:  _mm_prefetch((char*)(Source + 24), _MM_HINT_NTA);	// Fall through
            case 6:  _mm_prefetch((char*)(Source + 20), _MM_HINT_NTA);	// Fall through
            case 5:  _mm_prefetch((char*)(Source + 16), _MM_HINT_NTA);	// Fall through
            case 4:  _mm_prefetch((char*)(Source + 12), _MM_HINT_NTA);	// Fall through
            case 3:  _mm_prefetch((char*)(Source + 8), _MM_HINT_NTA);	// Fall through
            case 2:  _mm_prefetch((char*)(Source + 4), _MM_HINT_NTA);	// Fall through
            case 1:  _mm_prefetch((char*)(Source + 0), _MM_HINT_NTA);	// Fall through

                                                                        // Do four quadwords per loop to minimize stalls.
                for (size_t i = CacheLines; i > 0; --i)
                {
                    // If this is a large copy, start prefetching future cache lines.  This also prefetches the
                    // trailing quadwords that are not part of a whole cache line.
                    if (i >= 10)
                        _mm_prefetch((char*)(Source + 40), _MM_HINT_NTA);

                    _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));
                    _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));
                    _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));
                    _mm_stream_si128(Dest + 3, _mm_load_si128(Source + 3));

                    Dest += 4;
                    Source += 4;
                }

            case 0:	// No whole cache lines to read
                break;
            }

            // Copy the remaining quadwords
            switch (NumQuadwords & 3)
            {
            case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));	 // Fall through
            case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));	 // Fall through
            case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));	 // Fall through
            default:
                break;
            }

            _mm_sfence();
#endif
        }

        void MemFill(void* __restrict _Dest, V4F FillVector, size_t NumQuadwords)
        {
#if K3D_USE_SSE
//            assert(kMath::IsAligned(_Dest, 16));

            register const __m128i Source = _mm_castps_si128(FillVector);
            __m128i* __restrict Dest = (__m128i* __restrict)_Dest;

            switch (((size_t)Dest >> 4) & 3)
            {
            case 1: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
            case 2: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
            case 3: _mm_stream_si128(Dest++, Source); --NumQuadwords;	 // Fall through
            default:
                break;
            }

            size_t WholeCacheLines = NumQuadwords >> 2;

            // Do four quadwords per loop to minimize stalls.
            while (WholeCacheLines--)
            {
                _mm_stream_si128(Dest++, Source);
                _mm_stream_si128(Dest++, Source);
                _mm_stream_si128(Dest++, Source);
                _mm_stream_si128(Dest++, Source);
            }

            // Copy the remaining quadwords
            switch (NumQuadwords & 3)
            {
            case 3: _mm_stream_si128(Dest++, Source);	 // Fall through
            case 2: _mm_stream_si128(Dest++, Source);	 // Fall through
            case 1: _mm_stream_si128(Dest++, Source);	 // Fall through
            default:
                break;
            }

            _mm_sfence();
#endif
        }
    }
}