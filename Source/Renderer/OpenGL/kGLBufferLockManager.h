#pragma once
#include <GL/glew.h>

struct BufferRange
{
    uint32 mStartOffset;
    uint32 mLength;

    bool Overlaps(const BufferRange& _rhs) const {
        return mStartOffset < (_rhs.mStartOffset + _rhs.mLength)
            && _rhs.mStartOffset < (mStartOffset + mLength);
    }
};

// --------------------------------------------------------------------------------------------------------------------
struct BufferLock
{
    BufferRange mRange;
    GLsync mSyncObj;
};

// --------------------------------------------------------------------------------------------------------------------
class kGLBufferLockManager
{
public:
    kGLBufferLockManager(bool _cpuUpdates);
    ~kGLBufferLockManager();

    void WaitForLockedRange(uint32 _lockBeginBytes, uint32 _lockLength);
    void LockRange(uint32 _lockBeginBytes, uint32 _lockLength);

private:
    void wait(GLsync* _syncObj);
    void cleanup(BufferLock* _bufferLock);

    std::vector<BufferLock> m_BufferLocks;

    // Whether it's the CPU (true) that updates, or the GPU (false)
    bool m_CPUUpdates;
};
