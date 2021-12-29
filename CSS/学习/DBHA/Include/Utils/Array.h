//
//  "$Id$"
//
//  Copyright (c)1992-2011, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

///	\file Utils/Array.h
///	从 boost.array 移植过来的包装 C/C++ 原生数组的容器.
///

#ifndef UTILS_ARRAY_H__
#define UTILS_ARRAY_H__


#include <cassert>
#include <iterator>
#include <algorithm>
#include <stdexcept>


namespace Dahua {
namespace Utils {


/// \brief A standard container for storing a fixed size sequence of elements.
///
///	\ingroup sequences
///
///	Sets support random access iterators.
///
/// \param T Type of element. Required to be a complete type.
/// \param N Number of elements.
///
template<typename T, std::size_t N>
class TArray
{
public:
	// Support for zero-sized arrays mandatory.
	T elems[N];

	typedef T										value_type;
	typedef value_type*								pointer;
	typedef const value_type*						const_pointer;
	typedef value_type&								reference;
	typedef const value_type&						const_reference;
	typedef value_type*								iterator;
	typedef const value_type*						const_iterator;
	typedef std::size_t								size_type;
	typedef std::ptrdiff_t							difference_type;
	typedef std::reverse_iterator<iterator>			reverse_iterator;
	typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;

	// No explicit construct/copy/destroy for aggregate type.

	void assign(const value_type& __u)
	{ std::fill_n(begin(), size(), __u); }

	void swap(TArray& __other)
	{ std::swap_ranges(begin(), end(), __other.begin()); }

	// Iterators.
	iterator begin()
	{ return iterator(&elems[0]); }

	const_iterator begin() const
	{ return const_iterator(&elems[0]); }

	iterator end()
	{ return iterator(&elems[N]); }

	const_iterator end() const
	{ return const_iterator(&elems[N]); }

	reverse_iterator rbegin()
	{ return reverse_iterator(end()); }

	const_reverse_iterator rbegin() const
	{ return const_reverse_iterator(end()); }

	reverse_iterator rend()
	{ return reverse_iterator(begin()); }

	const_reverse_iterator rend() const
	{ return const_reverse_iterator(begin()); }

	const_iterator cbegin() const
	{ return const_iterator(&elems[0]); }

	const_iterator cend() const
	{ return const_iterator(&elems[N]); }

	const_reverse_iterator crbegin() const
	{ return const_reverse_iterator(end()); }

	const_reverse_iterator crend() const
	{ return const_reverse_iterator(begin()); }

	// Capacity.
	static size_type size() { return N; }

	static bool empty() { return false; }

	static size_type max_size() { return N; }

	enum { static_size = N };

	// Element access.
	reference operator[](size_type i)
	{
		assert(i < N && "out of range");
		return elems[i];
	}

	const_reference operator[](size_type i) const
	{
		assert(i < N && "out of range");
		return elems[i];
	}

	reference at(size_type i)
	{
		if (i >= N) throw std::out_of_range("TArray<>: index out of range");
		return elems[i];
	}

	const_reference at(size_type i) const
	{
		if (i >= N) throw std::out_of_range("TArray<>: index out of range");
		return elems[i];
	}

	reference front()
	{ return *begin(); }

	const_reference front() const
	{ return *begin(); }

	reference back()
	{ return &elems[N - 1]; }

	const_reference back() const
	{ return &elems[N - 1]; }

	pointer data()
	{ return &elems[0]; }

	const_pointer data() const
	{ return &elems[0]; }
};

/// 特化没有元素的TArray模板
template<typename T>
class TArray< T, 0 >
{
public:
	typedef T										value_type;
	typedef value_type*								pointer;
	typedef const value_type*						const_pointer;
	typedef value_type&								reference;
	typedef const value_type&						const_reference;
	typedef value_type*								iterator;
	typedef const value_type*						const_iterator;
	typedef std::size_t								size_type;
	typedef std::ptrdiff_t							difference_type;
	typedef std::reverse_iterator<iterator>			reverse_iterator;
	typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;

	// assign one value to all elements
	void assign (const T& ) {}

	void swap (TArray<T,0>& y) {}

	// iterator support
	iterator begin()
	{ return iterator(reinterpret_cast<T*>(this)); }

	const_iterator begin() const
	{ return const_iterator(reinterpret_cast<T const*>(this)); }

	iterator end()
	{ return begin(); }

	const_iterator end() const
	{ return begin(); }

	reverse_iterator rbegin()
	{ return reverse_iterator(end()); }

	const_reverse_iterator rbegin() const
	{ return const_reverse_iterator(end()); }

	reverse_iterator rend()
	{ return reverse_iterator(begin()); }

	const_reverse_iterator rend() const
	{ return const_reverse_iterator(begin()); }

	const_iterator cbegin() const
	{ return const_iterator(begin()); }

	const_iterator cend() const
	{ return const_iterator(end()); }

	const_reverse_iterator crbegin() const
	{ return const_reverse_iterator(end()); }

	const_reverse_iterator crend() const
	{ return const_reverse_iterator(begin()); }

	// size is constant
	static size_type size() { return 0; }

	static bool empty() { return true; }

	static size_type max_size() { return 0; }

	enum { static_size = 0 };

	// operator[]
	reference operator[](size_type i)
	{ return failed_rangecheck(); }

	const_reference operator[](size_type i) const
	{ return failed_rangecheck(); }

	// at() with range check
	reference at(size_type i)
	{ return failed_rangecheck(); }

	const_reference at(size_type i) const
	{ return failed_rangecheck(); }

	// front() and back()
	reference front()
	{ return failed_rangecheck(); }

	const_reference front() const
	{ return failed_rangecheck(); }

	reference back()
	{ return failed_rangecheck(); }

	const_reference back() const
	{ return failed_rangecheck(); }

	// direct access to data (read-only)
	const T* data() const { return 0; }

	T* data() { return 0; }

private:
	// check range (may be private because it is static)
	static reference failed_rangecheck ()
	{
		throw std::out_of_range("attempt to access element of an empty array");

		//
		// We need to return something here to keep
		// some compilers happy: however we will never
		// actually get here....
		//
		static T placeholder;
		return placeholder;
	}
};

} // namespace Utils

// TArray comparisons.
template<typename T, std::size_t N>
inline bool operator==(Utils::TArray<T, N> const& x, Utils::TArray<T, N> const& y)
{ return std::equal(x.begin(), x.end(), y.begin()); }

template<typename T, std::size_t N>
inline bool operator!=(Utils::TArray<T, N> const& x, Utils::TArray<T, N> const& y)
{ return !(x == y); }

template<typename T, std::size_t N>
inline bool operator<(Utils::TArray<T, N> const& x, Utils::TArray<T, N> const& y)
{ return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end()); }

template<typename T, std::size_t N>
inline bool operator>(Utils::TArray<T, N> const& x, Utils::TArray<T, N> const& y)
{ return y < x; }

template<typename T, std::size_t N>
inline bool operator<=(Utils::TArray<T, N> const& x, Utils::TArray<T, N> const& y)
{ return !(x > y); }

template<typename T, std::size_t N>
inline bool operator>=(Utils::TArray<T, N> const& x, Utils::TArray<T, N> const& y)
{ return !(x < y); }

// global swap()
template<typename T, std::size_t N>
inline void swap(Utils::TArray<T, N>& x, Utils::TArray<T, N>& y)
{ x.swap(y); }


} // namespace Dahua


#endif // UTILS_ARRAY_H__
