#pragma once
#ifndef __ngfx_container_h__
#define __ngfx_container_h__

#include <map>
#include <unordered_map>
#include <vector>
#include <type_traits>
#include <ngfx_allocator.h>

namespace ngfx
{
	template <typename key_type, typename value_type>
	class HashMap
	{
		typedef std::unordered_map<key_type, value_type> this_map;
	public:
		HashMap() {}
		~HashMap() {}

		bool contains(key_type const& key) const
		{
			return m_map.find(key) != m_map.end();
		}

	private:
		this_map m_map;
	};

	template <typename value_type, class allocator_type = std::allocator<value_type> >
	class Vec
	{
		typedef std::vector<value_type, allocator_type> this_Vec;
        
	public:
        typedef typename std::vector<value_type, allocator_type>::iterator iterator;
		Vec() {}
		~Vec() {}

        iterator begin() {
            return m_Vec.begin();
        }

        iterator end() {
            return m_Vec.end();
        }

        void push(value_type const& val) {
            m_Vec.push_back(val);
        }

        void push(Vec<value_type> const& other) 
        {
            other.iter([this](value_type const& v) 
            {
                this->push(v);
            });
        }

        void push(value_type && val) {
            m_Vec.push_back(std::forward<value_type>(val));
        }

        void pop() {
            m_Vec.pop_back();
        }

        template <typename iter_fn>
        void iter(iter_fn fn_iter) const
        {
            for (value_type const& v : m_Vec)
            {
                fn_iter(v);
            }
        }

        template <typename iter_fn>
        void iter_mut(iter_fn fn_iter)
        {
            for (value_type& v : m_Vec)
            {
                fn_iter(v);
            }
        }

        inline bool empty() const { return m_Vec.empty(); }

        void clear()
        {
            m_Vec.clear();
        }

        void resize(size_t num) {
            m_Vec.resize(num);
        }

        size_t num() const { return m_Vec.size(); }

        const value_type& at(size_t i) const { return m_Vec[i]; }
        value_type& at(size_t i) { return m_Vec[i]; }

        size_t add_uninitialized()
        {
            size_t id = m_Vec.size();
            m_Vec.push_back(value_type());
            return id;
        }

		value_type& operator[] (size_t index) { return m_Vec[index]; }

		const value_type& operator[] (size_t index) const { return m_Vec[index]; }

	private:
		this_Vec m_Vec;
	};

    template <typename value_type>
    class VecUPtr : public Vec<UniqPtr<value_type> >
    {
    public:
        constexpr VecUPtr() = default;
    };
}

template <typename value_type, class allocator_type>
void* operator new(size_t size, ngfx::Vec<value_type, allocator_type>& inVec) {
    size_t id = inVec.add_uninitialized();
    return &inVec.at(id);
}
#endif
