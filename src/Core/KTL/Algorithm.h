#pragma once

#ifndef __k3d_Algorithm_h__
#define __k3d_Algorithm_h__

namespace k3d
{
    template <class TContainer>
    struct ContainerHelper
    {
        typedef typename TContainer::Iterator Iterator;
        typedef typename TContainer::ValueType ValueType;
        static Iterator BinarySearch(Iterator Begin, Iterator End, ValueType Value)
        {
            while (Begin < End) 
            {
                Iterator Mid = Begin + (End - Begin) >> 1;
                ValueType MidVal = *Mid;
                if (MidVal < Value) 
                {
                    Begin = Mid + 1;
                }
                else if (MidVal > Value) 
                {
                    End = Mid - 1;
                }
                else 
                {
                    return Mid;  // value found
                }
            }
            return End;  // value not present
        }
    };
}

#endif