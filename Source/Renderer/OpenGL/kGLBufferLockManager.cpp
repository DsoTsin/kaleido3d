#include "kGLBufferLockManager.h"

GLuint64 kOneSecondInNanoSeconds = 1000000000;

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
kGLBufferLockManager::kGLBufferLockManager(bool _cpuUpdates)
: m_CPUUpdates(_cpuUpdates)
{

}

// --------------------------------------------------------------------------------------------------------------------
kGLBufferLockManager::~kGLBufferLockManager()
{
    for (auto it = m_BufferLocks.begin(); it != m_BufferLocks.end(); ++it) {
        cleanup(&*it);
    }

    m_BufferLocks.clear();
}

// --------------------------------------------------------------------------------------------------------------------
void kGLBufferLockManager::WaitForLockedRange(uint32 _lockBeginBytes, uint32 _lockLength)
{
    BufferRange testRange = { _lockBeginBytes, _lockLength };
    std::vector<BufferLock> swapLocks;
    for (auto it = m_BufferLocks.begin(); it != m_BufferLocks.end(); ++it)
    {
        if (testRange.Overlaps(it->mRange)) {
            wait(&it->mSyncObj);
            cleanup(&*it);
        } else {
            swapLocks.push_back(*it);
        }
    }

    m_BufferLocks.swap(swapLocks);
}

// --------------------------------------------------------------------------------------------------------------------
void kGLBufferLockManager::LockRange(uint32 _lockBeginBytes, uint32 _lockLength)
{
    BufferRange newRange = { _lockBeginBytes, _lockLength };
    GLsync syncName = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    BufferLock newLock = { newRange, syncName };

    m_BufferLocks.push_back(newLock);
}

// --------------------------------------------------------------------------------------------------------------------
void kGLBufferLockManager::wait(GLsync* _syncObj)
{
    if  (m_CPUUpdates) {
        GLbitfield waitFlags = 0;
        GLuint64 waitDuration = 0;
        while (1) {
            GLenum waitRet = glClientWaitSync(*_syncObj, waitFlags, waitDuration);
            if (waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED) {
                return;
            }

            if (waitRet == GL_WAIT_FAILED) {
                assert(!"Not sure what to do here. Probably raise an exception or something.");
                return;
            }

            // After the first time, need to start flushing, and wait for a looong time.
            waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
            waitDuration = kOneSecondInNanoSeconds;
        }
    } else {
        glWaitSync(*_syncObj, 0, GL_TIMEOUT_IGNORED);
    }
}

// --------------------------------------------------------------------------------------------------------------------
void kGLBufferLockManager::cleanup(BufferLock* _bufferLock)
{
    glDeleteSync(_bufferLock->mSyncObj);
}
