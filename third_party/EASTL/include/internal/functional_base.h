/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_INTERNAL_FUNCTIONAL_BASE_H
#define EASTL_INTERNAL_FUNCTIONAL_BASE_H

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif

#include <EASTL/internal/config.h>
#include <EASTL/internal/memory_base.h>
#include <EASTL/internal/move_help.h>
#include <EASTL/type_traits.h>

namespace std
{
	// foward declaration for swap
	template <typename T>
	inline void swap(T& a, T& b) EA_NOEXCEPT_IF(std::is_nothrow_move_constructible<T>::value &&
	std::is_nothrow_move_assignable<T>::value);


	/// invoke
	///
	/// invoke is a generalized function-call operator which works on function pointers, member function
	/// pointers, callable objects and member pointers.
	///
	/// For (member/non-member) function pointers and callable objects, it returns the result of calling
	/// the function/object with the specified arguments. For member data pointers, it simply returns
	/// the member.
	///
	/// Note that there are also reference_wrapper specializations of invoke, which need to be defined
	/// later since reference_wrapper uses invoke in its implementation. Those are defined immediately
	/// after the definition of reference_wrapper.
	///
	/// http://en.cppreference.com/w/cpp/utility/functional/invoke
	///
	template <typename R, typename C, typename T, typename... Args>
	auto invoke_impl(R C::*func, T&& obj, Args&&... args) ->
	typename enable_if<
		is_base_of<C, decay_t<decltype(obj)>>::value,
		decltype((std::forward<T>(obj).*func)(std::forward<Args>(args)...))
	>::type
	{
		return (std::forward<T>(obj).*func)(std::forward<Args>(args)...);
	}

	template <typename F, typename... Args>
	auto invoke_impl(F&& func, Args&&... args) -> decltype(std::forward<F>(func)(std::forward<Args>(args)...))
	{
		return std::forward<F>(func)(std::forward<Args>(args)...);
	}


	template <typename R, typename C, typename T, typename... Args>
	auto invoke_impl(R C::*func, T&& obj, Args&&... args) -> decltype(((*std::forward<T>(obj)).*func)(std::forward<Args>(args)...))
	{
		return ((*std::forward<T>(obj)).*func)(std::forward<Args>(args)...);
	}

	template <typename M, typename C, typename T>
	auto invoke_impl(M C::*member, T&& obj) ->
	typename enable_if<
		is_base_of<C, decay_t<decltype(obj)>>::value,
		decltype(obj.*member)
	>::type
	{
		return obj.*member;
	}

	template <typename M, typename C, typename T>
	auto invoke_impl(M C::*member, T&& obj) -> decltype((*std::forward<T>(obj)).*member)
	{
		return (*std::forward<T>(obj)).*member;
	}

	template <typename F, typename... Args>
	inline decltype(auto) invoke(F&& func, Args&&... args)
	{
		return invoke_impl(std::forward<F>(func), std::forward<Args>(args)...);
	}

	template <typename F, typename = void, typename... Args>
	struct invoke_result_impl {
	};

	template <typename F, typename... Args>
	struct invoke_result_impl<F, void_t<decltype(invoke_impl(std::declval<decay_t<F>>(), std::declval<Args>()...))>, Args...>
	{
		typedef decltype(invoke_impl(std::declval<decay_t<F>>(), std::declval<Args>()...)) type;
	};

	template <typename F, typename... Args>
	struct invoke_result : public invoke_result_impl<F, void, Args...> {};

	#if !defined(EA_COMPILER_NO_TEMPLATE_ALIASES)
		template <typename F, typename... Args>
		using invoke_result_t = typename invoke_result<F, Args...>::type;
	#endif

	template <typename F, typename = void, typename... Args>
	struct is_invocable_impl : public std::false_type {};

	template <typename F, typename... Args>
	struct is_invocable_impl<F, void_t<typename std::invoke_result<F, Args...>::type>, Args...> : public std::true_type {};

	template <typename F, typename... Args>
	struct is_invocable : public is_invocable_impl<F, void, Args...> {};

	template <typename R, typename F, typename = void, typename... Args>
	struct is_invocable_r_impl : public std::false_type {};

	template <typename R, typename F, typename... Args>
	struct is_invocable_r_impl<R, F, void_t<typename invoke_result<F, Args...>::type>, Args...>
		: public is_convertible<typename invoke_result<F, Args...>::type, R> {};

	template <typename R, typename F, typename... Args>
	struct is_invocable_r : public is_invocable_r_impl<R, F, void, Args...> {};

	#if EASTL_VARIABLE_TEMPLATES_ENABLED
		template <typename F, typename... Args>
		EASTL_CPP17_INLINE_VARIABLE EA_CONSTEXPR bool is_invocable_v = is_invocable<F, Args...>::value;

		template <typename R, typename F, typename... Args>
		EASTL_CPP17_INLINE_VARIABLE EA_CONSTEXPR bool is_invocable_r_v = is_invocable_r<R, F, Args...>::value;
	#endif

	/// allocator_arg_t
	///
	/// allocator_arg_t is an empty class type used to disambiguate the overloads of
	/// constructors and member functions of allocator-aware objects, including tuple,
	/// function, promise, and packaged_task.
	/// http://en.cppreference.com/w/cpp/memory/allocator_arg_t
	///
	struct allocator_arg_t
	{};


	/// allocator_arg
	///
	/// allocator_arg is a constant of type allocator_arg_t used to disambiguate, at call site,
	/// the overloads of the constructors and member functions of allocator-aware objects,
	/// such as tuple, function, promise, and packaged_task.
	/// http://en.cppreference.com/w/cpp/memory/allocator_arg
	///
	#if !defined(EA_COMPILER_NO_CONSTEXPR)
		EA_CONSTEXPR allocator_arg_t allocator_arg = allocator_arg_t();
	#endif


	template <typename Argument, typename Result>
	struct unary_function
	{
		typedef Argument argument_type;
		typedef Result   result_type;
	};


	template <typename Argument1, typename Argument2, typename Result>
	struct binary_function
	{
		typedef Argument1 first_argument_type;
		typedef Argument2 second_argument_type;
		typedef Result    result_type;
	};


	/// less<T>
	template <typename T = void>
	struct less : public binary_function<T, T, bool>
	{
		EA_CPP14_CONSTEXPR bool operator()(const T& a, const T& b) const
			{ return a < b; }
	};

	// http://en.cppreference.com/w/cpp/utility/functional/less_void
	template <>
	struct less<void>
	{
		template<typename A, typename B>
		EA_CPP14_CONSTEXPR auto operator()(A&& a, B&& b) const
			-> decltype(std::forward<A>(a) < std::forward<B>(b))
			{ return std::forward<A>(a) < std::forward<B>(b); }
	};


	/// reference_wrapper
	template <typename T>
	class reference_wrapper
	{
	public:
		typedef T type;

		reference_wrapper(T&) EA_NOEXCEPT;
		#if !defined(EA_COMPILER_NO_DELETED_FUNCTIONS)
			reference_wrapper(T&&) = delete;
		#endif
		reference_wrapper(const reference_wrapper<T>& x) EA_NOEXCEPT;

		reference_wrapper& operator=(const reference_wrapper<T>& x) EA_NOEXCEPT;

		operator T& () const EA_NOEXCEPT;
		T& get() const EA_NOEXCEPT;

		#if EASTL_VARIADIC_TEMPLATES_ENABLED
			template <typename... ArgTypes>
			typename std::result_of<T&(ArgTypes&&...)>::type operator() (ArgTypes&&...) const;
		#endif

	private:
		T* val;
	};

	template <typename T>
	reference_wrapper<T>::reference_wrapper(T &v) EA_NOEXCEPT
		: val(addressof(v))
	{}

	template <typename T>
	reference_wrapper<T>::reference_wrapper(const reference_wrapper<T>& other) EA_NOEXCEPT
		: val(other.val)
	{}

	template <typename T>
	reference_wrapper<T>& reference_wrapper<T>::operator=(const reference_wrapper<T>& other) EA_NOEXCEPT
	{
		val = other.val;
		return *this;
	}

	template <typename T>
	reference_wrapper<T>::operator T&() const EA_NOEXCEPT
	{
		return *val;
	}

	template <typename T>
	T& reference_wrapper<T>::get() const EA_NOEXCEPT
	{
		return *val;
	}

	template <typename T>
	template <typename... ArgTypes>
	typename std::result_of<T&(ArgTypes&&...)>::type reference_wrapper<T>::operator() (ArgTypes&&... args) const
	{
		return std::invoke(*val, std::forward<ArgTypes>(args)...);
	}

	// reference_wrapper-specific utilties
	template <typename T>
	reference_wrapper<T> ref(T& t) EA_NOEXCEPT;

	template <typename T>
	void ref(const T&&) = delete;

	template <typename T>
	reference_wrapper<T> ref(reference_wrapper<T>t) EA_NOEXCEPT;

	template <typename T>
	reference_wrapper<const T> cref(const T& t) EA_NOEXCEPT;

	template <typename T>
	void cref(const T&&) = delete;

	template <typename T>
	reference_wrapper<const T> cref(reference_wrapper<T> t) EA_NOEXCEPT;


	// reference_wrapper-specific type traits
	template <typename T>
	struct is_reference_wrapper_helper
		: public std::false_type {};

	template <typename T>
	struct is_reference_wrapper_helper<std::reference_wrapper<T> >
		: public std::true_type {};

	template <typename T>
	struct is_reference_wrapper
		: public std::is_reference_wrapper_helper<typename std::remove_cv<T>::type> {};


	// Helper which adds a reference to a type when given a reference_wrapper of that type.
	template <typename T>
	struct remove_reference_wrapper
		{ typedef T type; };

	template <typename T>
	struct remove_reference_wrapper< std::reference_wrapper<T> >
		{ typedef T& type; };

	template <typename T>
	struct remove_reference_wrapper< const std::reference_wrapper<T> >
		{ typedef T& type; };

	// reference_wrapper specializations of invoke
	// These have to come after reference_wrapper is defined, but reference_wrapper needs to have a
	// definition of invoke, so these specializations need to come after everything else has been defined.
	template <typename R, typename C, typename T, typename... Args>
	auto invoke_impl(R (C::*func)(Args...), T&& obj, Args&&... args) ->
		typename enable_if<is_reference_wrapper<typename remove_reference<T>::type>::value,
						   decltype((obj.get().*func)(std::forward<Args>(args)...))>::type
	{
		return (obj.get().*func)(std::forward<Args>(args)...);
	}

	template <typename M, typename C, typename T>
	auto invoke_impl(M(C::*member), T&& obj) ->
	    typename enable_if<is_reference_wrapper<typename remove_reference<T>::type>::value,
	                       decltype(obj.get().*member)>::type
	{
		return obj.get().*member;
	}


	///////////////////////////////////////////////////////////////////////
	// bind
	///////////////////////////////////////////////////////////////////////

	/// bind1st
	///
	template <typename Operation>
	class binder1st : public unary_function<typename Operation::second_argument_type, typename Operation::result_type>
	{
		protected:
			typename Operation::first_argument_type value;
			Operation op;

		public:
			binder1st(const Operation& x, const typename Operation::first_argument_type& y)
				: value(y), op(x) { }

			typename Operation::result_type operator()(const typename Operation::second_argument_type& x) const
				{ return op(value, x); }

			typename Operation::result_type operator()(typename Operation::second_argument_type& x) const
				{ return op(value, x); }
	};


	template <typename Operation, typename T>
	inline binder1st<Operation> bind1st(const Operation& op, const T& x)
	{
		typedef typename Operation::first_argument_type value;
		return binder1st<Operation>(op, value(x));
	}


	/// bind2nd
	///
	template <typename Operation>
	class binder2nd : public unary_function<typename Operation::first_argument_type, typename Operation::result_type>
	{
		protected:
			Operation op;
			typename Operation::second_argument_type value;

		public:
			binder2nd(const Operation& x, const typename Operation::second_argument_type& y)
				: op(x), value(y) { }

			typename Operation::result_type operator()(const typename Operation::first_argument_type& x) const
				{ return op(x, value); }

			typename Operation::result_type operator()(typename Operation::first_argument_type& x) const
				{ return op(x, value); }
	};


	template <typename Operation, typename T>
	inline binder2nd<Operation> bind2nd(const Operation& op, const T& x)
	{
		typedef typename Operation::second_argument_type value;
		return binder2nd<Operation>(op, value(x));
	}

} // namespace std

#endif // Header include guard
