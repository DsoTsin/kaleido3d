#pragma once

K3D_COMMON_NS
{
template<class T>
struct __RemoveRef
{
	typedef T Type;
};
template<class T>
struct __RemoveRef<T&>
{
	typedef T Type;
};
template<class T>
struct __RemoveRef<T&&>
{
	typedef T Type;
};
template<class T>
inline constexpr typename __RemoveRef<T>::Type&& Move(T&& arg)
{
	return static_cast<typename __RemoveRef<T>::Type&&>(arg);
}
template<class T>
inline constexpr T&& Forward(typename __RemoveRef<T>::Type& arg)
{
	return static_cast<T&&>(arg);
}

template<class T>
inline constexpr T&& Forward(typename __RemoveRef<T>::Type&& arg)
{
	//static_assert(!is_lvalue_reference<T>::value, "bad forward call");
	return (static_cast<T&&>(arg));
}
}