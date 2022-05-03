#pragma once

#include <lux/base_core.h>

#include <tuple>
#include <functional>

namespace lux
{

	template< class Class, class Ret, class... Args >
	class method_ptr
	{
		template< class Cy, class My, class = decltype((*std::declval<const Cy*>().*std::declval<My>())(std::declval<Args>()...)) >
		static std::true_type _test_const(int);
		template< class Cy, class My >
		static std::false_type _test_const(...);

	public:
		using return_type = Ret;
		using args_tuple = std::tuple<Args...>;
		using class_type = Class;

		using method_type = return_type(class_type::*)(Args...);

		constexpr ~method_ptr() = default;

		constexpr method_ptr()
			: _class(nullptr), _method(nullptr) {
		}
		constexpr method_ptr(class_type* clss, method_type method)
			: _class(clss), _method(method) {
		}

		constexpr operator bool() const noexcept {
			return _method != nullptr;
		}
		constexpr return_type operator()(Args&&... args) {
			return (*_class.*_method)(std::forward<Args>(args)...);
		}

		class_type* get_class() {
			return _class;
		}
		class_type* set_class(class_type* clss) {
			return _class = clss;
		}

		method_type get_method() {
			return _method;
		}
		method_type set_method(method_type method) {
			return _method = method;
		}

		void swap(method_ptr& other) {
			std::swap(_class, other._class);
			std::swap(_method, other._method);
		}
		std::function<return_type(Args...)> bind() const {
			return std::bind_front(_method, _class);
		}

	private:
		class_type* _class;
		method_type _method;
	};

	template< class Class, class Ret, class... Args >
	class const_method_ptr
	{
	public:
		using return_type = Ret;
		using args_tuple = std::tuple<Args...>;
		using class_type = Class;

		using method_type = return_type(class_type::*)(Args...) const;

		constexpr ~const_method_ptr() = default;

		constexpr const_method_ptr()
			: _class(nullptr), _method(nullptr) {
		}
		constexpr const_method_ptr(class_type* clss, method_type method)
			: _class(clss), _method(method) {
		}

		constexpr operator bool() const noexcept {
			return _method != nullptr;
		}
		constexpr return_type operator()(Args&&... args) noexcept(noexcept((*_class.*_method)(std::forward<Args>(args)...))) {
			return (*_class.*_method)(std::forward<Args>(args)...);
		}

		class_type* get_class() {
			return _class;
		}
		class_type* set_class(class_type* clss) {
			return _class = clss;
		}

		method_type get_method() {
			return _method;
		}
		method_type set_method(method_type method) {
			return _method = method;
		}

		void swap(const_method_ptr& other) {
			std::swap(_class, other._class);
			std::swap(_method, other._method);
		}
		std::function<return_type(Args...)> bind() const {
			return std::bind_front(_method, _class);
		}

	private:
		class_type* _class;
		method_type _method;
	};

}
