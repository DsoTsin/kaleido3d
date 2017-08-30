
#pragma once

#ifndef __intrusive_list_h__
#define __intrusive_list_h__

#include <Config/Prerequisities.h>
#include <memory>
#include <list>

K3D_COMMON_NS
{

  class _My_Intrusive_List_Base;
  class _My_Intrusive_List_Node_Base;

  class _My_Intrusive_List_Node_Base
  {
  public:
    _My_Intrusive_List_Node_Base() : m_Prev( NULL ), m_Next( NULL ), m_OwningLst( NULL ) {}
    virtual ~_My_Intrusive_List_Node_Base()
    {
    }

    _My_Intrusive_List_Node_Base* prev() { return m_Prev; }
    _My_Intrusive_List_Node_Base* next() { return m_Next; }
    _My_Intrusive_List_Base* GetOwningList() { return m_OwningLst; }

    friend class _My_Intrusive_List_Base;

  private:
    _My_Intrusive_List_Node_Base  *m_Prev;
    _My_Intrusive_List_Node_Base  *m_Next;

    _My_Intrusive_List_Base       *m_OwningLst;
  };

  class _My_Intrusive_List_Base
  {
  public:
    typedef _My_Intrusive_List_Node_Base  _Node_Base;
    typedef _My_Intrusive_List_Base       _List_Base;
    typedef size_t                        size_type;

    _My_Intrusive_List_Base() : m_Head( NULL ), m_Tail( NULL ) {}

    bool        empty() { return m_Head == NULL; }
    void        clear() {}
    size_type   size() { return 0 ;}

  //protected:
    void prepend( _Node_Base *element ) 
    {
      _List_Base *list = element->GetOwningList();
      if ( list )
      {
        _Node_Base *_prev = element->m_Prev;
        _Node_Base *_next = element->m_Next;
        if ( _prev )
        {
          _prev->m_Next = _next;
          element->m_Prev = nullptr;
        }
        if ( _next )
        {
          _next->m_Prev = _prev;
          element->m_Next = nullptr;
        }
        if ( list->m_Head == element ) list->m_Head = _next;
        if ( list->m_Tail == element ) list->m_Tail = _prev;
      }
      // if m_head isn't null
      if ( m_Head )
      {
        m_Head->m_Prev = element;
        element->m_Next = m_Head;
        m_Head = element;
      }
      else
      {
        m_Head = m_Tail = element;
      }
      element->m_OwningLst = this;
    }

    void append( _Node_Base *element )
    {
      _List_Base *list = element->m_OwningLst;
      if ( list )
      {
        _Node_Base *_prev = element->m_Prev;
        _Node_Base *_next = element->m_Next;
        if ( _prev )
        {
          _prev->m_Next = _next;
          element->m_Prev = nullptr;
        }
        if ( _next )
        {
          _next->m_Prev = _prev;
          element->m_Next = nullptr;
        }
        if ( list->m_Head == element ) list->m_Head = _next;
        if ( list->m_Tail == element ) list->m_Tail = _prev;
      }
      if ( m_Tail )
      {
        m_Tail->m_Next = element;
        element->m_Prev = m_Tail;
        m_Tail = element;
      }
      else
      {
        m_Head = m_Tail = element;
      }
      element->m_OwningLst = this;
    }

    void insertBefore( _Node_Base *element, _Node_Base *before );
    void insertAfter( _Node_Base *element, _Node_Base *after );
    void erase( _Node_Base *element )
    {
      _Node_Base *_prev = element->m_Prev;
      _Node_Base *_next = element->m_Next;
      if ( _prev ) _prev->m_Next = _next;
      if ( _next ) _next->m_Prev = _prev;
      if ( m_Head == element ) m_Head = _next;
      if ( m_Tail == element ) m_Tail = _prev;
      element->m_Prev = nullptr;
      element->m_Next = nullptr;
      element->m_OwningLst = nullptr;
    }

    _Node_Base* _Next( _Node_Base *node ) { return node->next(); }
    _Node_Base* _Prev( _Node_Base *node ) { return node->prev(); }

    friend class _My_Intrusive_List_Node_Base;
  private:
    _My_Intrusive_List_Node_Base  *m_Head;
    _My_Intrusive_List_Node_Base  *m_Tail;
  };

  template <class _My_List>
  class IntruLstNodeIter
  {
  public:
    typedef typename _My_List::_My_Node_Type  _Node_Type;
    typedef _Node_Type *                      _Node_Ptr;
    typedef IntruLstNodeIter<_My_List>        _My_Iter;

    typedef _Node_Ptr                         pointer;
    typedef _Node_Type &                      reference;

    IntruLstNodeIter() : m_Ptr( NULL ) {}
    IntruLstNodeIter( _Node_Ptr _Pnode, const _My_List *_Plist ) : m_Ptr( _Pnode ) { m_LstPtr = _Plist; }

    reference   operator*() const { return (_My_List::_Myval( m_Ptr )); }
    pointer     operator->() const { return (::std::pointer_traits<pointer>::pointer_to( **this )); }
    _My_Iter &  operator++ () { m_Ptr = _My_List::_Next( m_Ptr ); return (*this); }
    _My_Iter    operator++ (int) { _My_Iter _Tmp = *this; ++*this; return (_Tmp); }
    _My_Iter&   operator--() { m_Ptr = _My_List::_Prev( m_Ptr ); return (*this); }
    _My_Iter    operator--(int) { _My_Iter _Tmp = *this; --*this; return (_Tmp); }
    bool        operator==(const _My_Iter& _Right) const { return (m_Ptr == _Right.m_Ptr); }
    bool        operator!=(const _My_Iter& _Right) const { return (!(*this == _Right)); }
    _Node_Ptr   _Mynode() const { return (m_Ptr); }

    _Node_Ptr m_Ptr;
    _My_List* m_LstPtr;
  };

  /// stl adapter
  template <class _Ty>
  class IntruLst;

  template <class _Ty>
  class IntruLstNode : public _My_Intrusive_List_Node_Base
  {
  public:
    typedef _My_Intrusive_List_Node_Base  _Node_Base;
    typedef IntruLst<_Ty>                 _Lst;
    _Ty *   prev( void ) const { return (static_cast<_Ty *>(static_cast<IntruLstNode<_Ty> *>(_Node_Base::prev()))); }
    _Ty *   next( void ) const { return (static_cast<_Ty *>(static_cast<IntruLstNode<_Ty> *>(_Node_Base::next()))); }
    _Lst *  GetOwningList( void ) const { return (static_cast<_Lst*>(_Node_Base::GetOwningList())); }

  protected:
    IntruLstNode();
  };

  template <class _Ty>
  class IntruLst : public _My_Intrusive_List_Base
  {
  public:
    typedef IntruLstNode<_Ty>          _My_Node_Type;
    typedef _My_Intrusive_List_Base::size_type  size_type;
    typedef IntruLstNodeIter< IntruLst<_Ty> >   iterator;

    iterator  begin() { return iterator( this->_Nextnode( this->m_Head ), this ); }
    iterator  end() { return iterator( this->m_Tail, this ); }
    bool      empty() { return _My_Intrusive_List_Base::empty(); }
    void      erase( iterator &iter ) { }
    void      clear();
    size_type size() const {}

  private:

  };
}

#endif
