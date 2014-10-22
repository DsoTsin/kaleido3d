/**********************************************
k3dVariant is for Streamable Class 
***********************************************/
#pragma once
#ifndef __k3dVariant_h__
#define __k3dVariant_h__
#include <Config/Prerequisities.h>
#include <Config/OSHeaders.h>

/// k3dVariant
/// \brief The k3dVariant class
/// A class data Wrapper For all types
class k3dVariant
{
public:
  inline k3dVariant()
    : m_DataSize( 0 ), m_DataPtr( NULL ), m_isDeepCopied( false )
  {}

  ~k3dVariant()
  {
    if (m_isDeepCopied )
      if ( m_DataPtr && m_DataSize != 0 )
      {
        ::free( m_DataPtr );
        m_DataPtr = NULL;
      }
  }
  
  inline void DeepCopy( void *data, int size )
  {
    if ( m_DataPtr == NULL )
    {
      m_DataPtr = malloc( size );
      m_DataSize = size;
    }
    ::memcpy( m_DataPtr, data, size );
    m_isDeepCopied = true;
  }

  inline k3dVariant( void *data, int size )
  {
    m_DataPtr = data; m_DataSize = size;
    m_isDeepCopied = false;
  }

  const size_t  Size() const { return m_DataSize; }
  const void*   ConstData() const { return static_cast<const void*>(m_DataPtr); }
  void*         Data()  { return m_DataPtr; }

private:
  size_t  m_DataSize;
  void*   m_DataPtr;
  bool    m_isDeepCopied;
};

#endif
