/**
 * MIT License
 *
 * Copyright (c) 2019 Zhou Qin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
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
                Iterator Mid = (Begin + (End - Begin)) >> 1;
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