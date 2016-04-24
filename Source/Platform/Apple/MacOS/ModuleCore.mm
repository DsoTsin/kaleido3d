#include <Core/ModuleCore.h>

#import <Foundation/Foundation.h>


namespace k3d
{
    namespace Core
    {
        kString GetExecutablePath() {
            NSString * pPath = [[NSBundle mainBundle] executablePath];
            return [pPath UTF8String];
        }
    }
}