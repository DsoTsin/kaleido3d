#pragma once
#include <type_traits>

namespace k3d
{
    class K3D_CORE_API Archive {
    public:
        Archive() : Handler(nullptr) {}
        virtual ~Archive() {}
        
        void SetIODevice(IIODevice * ioHandler) {
            Handler = ioHandler;
        }
        
        template <typename T>
        Archive & operator >> (T & data) {
            assert(std::is_pointer<T>::value != true && "cannot be serialize, not a pod class!!");
            Handler->Read((char*)&data, sizeof(T));
            return *this;
        }
        
        template <typename T>
        Archive & operator << (const T data) {
            assert(std::is_pointer<T>::value != true && "cannot be serialize, not a pod class!!");
            Handler->Write((U8*)&data, sizeof(T));
            return *this;
        }
        
        template <typename T>
        void ArrayIn(T *dataArray, size_t elemCount) {
            assert(std::is_pointer<T>::value != true && "ArrayIn Error: not a pod class");
            Handler->Write((U8*)dataArray, elemCount*sizeof(T));
        }
        
        template <typename T>
        void ArrayOut(T *dataArray, size_t elemCount) {
            assert(std::is_pointer<T>::value != true && "ArrayOut Error: not a pod class");
            Handler->Read((char*)dataArray, elemCount*sizeof(T));
        }
        
        virtual void FlushCurrentCache() {
            Handler->Flush();
        }
        
    protected:
        
        IIODevice* Handler;
    };
}
