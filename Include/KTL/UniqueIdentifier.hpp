#ifndef __UniqueIdentifier_hpp__
#define __UniqueIdentifier_hpp__
#pragma once
#include <set>
#include <map>

template <class Type> class UniqueIdentifier {

protected:

  explicit UniqueIdentifier(Type *i) : id(-1), instance(i) {
    id_types.insert(instance);
    set_id(create_id());
  }

  ~UniqueIdentifier() {
    id_types.erase(instance);
    id_type_map.erase(id);
  }

  static void ClearId() {
    last_id = 0;
  }
  int create_id() {
    int id = -1;
    do {
      id = last_id++;
      if(last_id >= 1073741824) last_id = 0;
    } while(id_type_map.find(id) != id_type_map.end());
    return id;
  }
  void set_id(int i) {
    typename std::map<int,Type*>::iterator it = id_type_map.find(i);
    if(id == i) {
      if(it != id_type_map.end() && it->second != instance) {
        it->second->id = create_id();
        id_type_map[it->second->id] = it->second;
        id_type_map[i] = instance;
      }
    } else {
      if(it != id_type_map.end()) {
        it->second->id = create_id();
        id_type_map[it->second->id] =it->second;
      }
      id_type_map.erase(id);
      id_type_map.erase(i); //???
      id = i;
    }
  }
  inline int GetId() const {
    return id;
  }

  static int IsInstance( Type* i) {
    Type * const & _i = i;
    return (id_types.find(_i) != id_types.end());
  }
  static Type *GetInstance(int id) {
    typename std::map<int,Type*>::iterator it = id_type_map.find(id);
    if(it == id_type_map.end()) return 0;
    return it->second;
  }

  typedef typename std::set<Type*>::iterator Iterator;

  static inline Iterator instances_begin() {
    return id_types.begin();
  }
  static inline Iterator instances_end() {
    return id_types.end();
  }
  static inline int instances_size() {
    return id_types.size();
  }

private:

  int id;
  Type *instance;

  static int last_id;
  static std::set< Type*       > id_types;
  static std::map< int,  Type* > id_type_map;
};

/*
 */
template <class Type> int UniqueIdentifier<Type>::last_id = 0;
template <class Type> std::set<Type*> UniqueIdentifier<Type>::id_types;
template <class Type> std::map<int,Type*> UniqueIdentifier<Type>::id_type_map;

#endif
