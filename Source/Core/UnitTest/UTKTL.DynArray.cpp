#include "Common.h"

#if K3DPLATFORM_OS_WIN
#pragma comment(linker,"/subsystem:console")
#endif

using namespace std;
using namespace k3d;

struct B
{
    B() : msg_(nullptr)
    {
        std::cout << "construct" << std::endl;
    }
    B(const char* msg) : msg_(msg)
    {
        std::cout << msg << std::endl;
    }
    
    B& operator=(const B& rhs)
    {
        std::cout << "rhs " << rhs.msg_
        << " assigned to " << std::endl;
        this->msg_ = rhs.msg_;
        return *this;
    }
    
    ~B()
    {
        std::cout << "deconstructB " << msg_ << std::endl;
    }
    const char *msg_;
};

void TestDynArrray()
{
    B b1{ "b1" }, b3{ "b3" };
    DynArray<int> ints;
    ints.Append(5).Append(6).Append(7).Append(0).Append(8);
    std::cout << ints[3] << std::endl;
    ints[3] = 0;
            
    std::cout << "for begin - end test start-----" << std::endl;
    for (auto& iter : ints)
    {
        std::cout << iter << std::endl;
    }
    std::cout << "for begin - end test end-----" << std::endl;
            
    std::cout << ints[3] << std::endl;
    std::cout << ints.Count() << std::endl;
    {
        B b2{ "b2" };
        DynArray<B> bs;
        bs.Append(b1)
        .Append(std::move(b2))
        .Append(b3);
        bs[2] = B("B modified");
    }
    {
        DynArray<B> bs;
        bs.Append(B("A1"))
			.Append(B("A2"))
			.Append(B("A3"))
			.Append(B("A4"))
			.Append(B("A5"));
    }
}

int main(int argc, char**argv)
{
	TestDynArrray();
	return 0;
}