#include "Public/ngfx.h"
#include "CoreMinimal.h"

static void log_print(int level, const char* msg)
{

}

typedef ngfx::Factory* (*fnCreate)(bool debug, decltype(log_print) call);

int main(int argc, char**argv) {
    k3d::os::LibraryLoader loader("ngfx_vk.dll");
    fnCreate create = (fnCreate)loader.ResolveSymbol("CreateFactory");
    create(true, log_print);
    return 0;
}