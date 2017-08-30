#pragma once
#include "Core/CoreMinimal.h"

namespace k3d
{
    namespace mobi
    {
        template <typename T>
        struct LogBuffer
        {
            typedef void (T::*NewLine)(String&& InLine);

            /*
             * Callback when new line formed 
             */
            LogBuffer(T* InArg, NewLine Callback, I32 SzBuf = 16384)
                : Data(nullptr)
                , OffSet(0)
                , Size(SzBuf)
                , OnNewLine(Callback)
                , Arg(InArg)
            {
                Data = new char[SzBuf];
            }

            ~LogBuffer()
            {
                delete[] Data;
            }

            LogBuffer& operator << (char c)
            {
                if (c == '\n')
                {
                    Data[OffSet] = '\0'; // End a line string
                    if (OffSet != 0 && OnNewLine)
                    {
                        String Copy(Data, OffSet);
                        (Arg->*OnNewLine)(Move(Copy));
                    }
                    OffSet = 0; // Reset Offset
                }
                else
                {
                    Data[OffSet] = c;
                    OffSet++;
                    if (OffSet == Size) // Reach the capacity, then truncate it
                    {
                        Data[OffSet - 1] = '\0';
                        if (OnNewLine)
                        {
                            String Copy(Data, OffSet);
                            (Arg->*OnNewLine)(Move(Copy));
                        }
                        OffSet = 0;
                    }
                }
                return *this;
            }

        private:
            char*       Data;
            I32         OffSet;
            I32         Size;
            NewLine     OnNewLine;
            T*          Arg;
        };
    }
}