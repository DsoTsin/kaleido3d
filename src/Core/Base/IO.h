#pragma once

#ifndef __k3d_IO_h__
#define __k3d_IO_h__

namespace k3d
{
    enum IOFlag : U8
    {
        Read                = 1,
        Write               = 1 << 1,
        ReadWrite           = (Read | Write),
        SnappyCompressed    = 1 << 2,
        // Asset, start with "asset://"
        AssetAccess         = 1 << 3,
    };

    struct K3D_CORE_API IIODevice
    {
        ~IIODevice() {}
        virtual bool      Open(const char* FileName, IOFlag Mode) = 0;
        virtual bool      IsEOF() = 0;
        virtual size_t    Read(char*, size_t) = 0;
        virtual size_t    Write(const void*, size_t) = 0;
        virtual bool      Seek(size_t offset) = 0;
        virtual bool      Skip(size_t offset) = 0;
        virtual void      Flush() = 0;
        virtual void      Close() = 0;
    };

    //KTYPE_META_TEMPLATE( IIODevice );

    template <class IsIODevice>
    IIODevice * GetIODevice()
    {
        //  assert(is_IIODevice<IsIODevice>::_YES);
        return IsIODevice::CreateIOInterface();
    }

}

#endif