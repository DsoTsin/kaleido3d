#include "CoreMinimal.h"
namespace k3d {
DynArray<U64> Values;

Handle Handle::New() { 
return Handle(); 
}

void Handle::Free(const Handle &InHandle) {

}

bool Handle::IsValid() const { return ~0ULL != m_Value; }
bool Handle::IsFreed() const { return false; }
Handle Handle::InvalidValue;
} // namespace k3d