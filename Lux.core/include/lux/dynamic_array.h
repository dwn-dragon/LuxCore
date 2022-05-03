#pragma once

#include <lux/base_core.h>

#include <lux/math.h>

#include <memory>

namespace lux
{

	template< class Ty, class Ay = std::allocator<Ty> >
	class dynamic_array
	{
	public:
		using value_type = Ty;
		using allocator_type = Ay;

		using size_type = size_t;
		using difference_type = ptrdiff_t;

		using reference = value_type&;
		using const_reference = const value_type&;

		using pointer = std::allocator_traits<allocator_type>::pointer;
		using const_pointer = std::allocator_traits<allocator_type>::const_pointer;

		using iterator = pointer;
		using const_iterator = const_pointer;

		using reverse_iterator = iterator;
		using const_reverse_iterator = const_iterator;

		constexpr ~dynamic_array() {
			if (!empty()) 
				clear();
		}

		constexpr explicit dynamic_array() noexcept
			: _alloc(), _size(0), _data(nullptr) {
		}
		constexpr explicit dynamic_array(const allocator_type& alloc) noexcept
			: _alloc(alloc), _size(0), _data(nullptr) {
		}

		constexpr dynamic_array(size_type count, const value_type& value, const allocator_type& alloc = allocator_type())
			: _alloc(alloc), _size(count), _data(nullptr) {
			if (_size > 0) {
				_data = _alloc_array(_size);
				for (auto it = begin(); it != end(); ++it)
					new (it) value_type(value);
			}
		}
		constexpr explicit dynamic_array(size_type count, const allocator_type& alloc = allocator_type()) 
			: _alloc(alloc), _size(count), _data(nullptr) {
			if (_size > 0) {
				_data = _alloc_array(_size);
				for (auto it = begin(); it != end(); ++it)
					new (it) value_type();
			}
		}

		template< class It >
		constexpr dynamic_array(It first, It last, const allocator_type& alloc = allocator_type())
			: _alloc(alloc), _size(std::distance(first, last)), _data(nullptr) {
			if (_size > 0) {
				_data = _alloc_array(_size);
				for (auto it = begin(); it != end(); ++it, ++first)
					new (it) value_type(*first);
			}
		}

		constexpr dynamic_array(std::initializer_list<value_type> init, const allocator_type& alloc = allocator_type())
			: _alloc(alloc), _size(init.size()), _data(nullptr) {
			if (_size > 0) {
				_data = _alloc_array(_size);
				auto _it = init.begin();
				for (auto it = begin(); it != end(); ++it, ++_it)
					new (it) value_type(*_it);
			}
		}

		constexpr dynamic_array(const dynamic_array& other) 
			: dynamic_array(other, other._alloc) {
		}
		constexpr dynamic_array(const dynamic_array& other, const allocator_type& alloc) 
			: _alloc(alloc), _size(other._size), _data(nullptr) {
			if (_size > 0) {
				_data = _alloc_array(_size);
				for (size_t i = 0; i < _size; i++)
					new (_data + i) value_type(other._data[i]);
			}
		}

		constexpr dynamic_array(dynamic_array&& other) noexcept
			: _alloc(other._alloc), _size(other._size), _data(other._data) {
			other._size = 0, other._data = nullptr;
		}
		constexpr dynamic_array(dynamic_array&& other, const allocator_type& alloc) 
			: _alloc(alloc), _size(other._size), _data(nullptr) {
			if (_size > 0) if (_alloc == other._alloc)
				_data = other._data;
			else {
				_data = _alloc_array(_size);
				for (size_t i = 0; i < _size; i++)
					new (_data + i) value_type(std::move(other._data[i]));
			}

			other._size = 0, other._data = nullptr;
		}

		constexpr dynamic_array& operator=(dynamic_array&& other) {
			if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
				if (!empty())
					clear();
				_alloc = std::move(other._alloc);
				_size = other._size, _data = other._data;
				other._size = 0, other._data = nullptr;
			}
			else if (_alloc == other._alloc) {
				if (!empty())
					clear();
				_size = other._size, _data = other._data;
				other._size = 0, other._data = nullptr;
			}
			else {
				if (other._size == _size) {
					for (size_t i = 0; i < _size; i++)
						_data[i] = std::move(other._data[i]);
				}
				else {
					if (!empty())
						clear();

					if (!other.empty()) {
						_size = other._size;
						_data = _alloc_array(_size);
						for (size_t i = 0; i < _size; i++)
							new (_data + i) value_type(std::move(other._data[i]));
					}
				}
			}

			return *this;
		}

		constexpr dynamic_array& operator=(const dynamic_array& other) {
			if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value) {
				auto old = _alloc;
				_alloc = other._alloc;

				if (_alloc != old) {
					// New alloc is different => Empty the array
					for (auto& e : (*this))
						e.~value_type();
					old.deallocate(_data, _size);

					_size = 0, _data = nullptr;
				}
			}

			if (other._size == _size) {
				// Same size => No reallocation the space
				for (size_t i = 0; i < _size; i++)
					_data[i] = other._data[i];
			}
			else {
				// Different size => Reallocation
				if (!empty)
					clear();

				if (!other.empty()) {
					_size = other._size;
					_data = _alloc_array(_size);
					for (size_t i = 0; i < _size; i++)
						new (_data + i) value_type(other._data[i]);
				}
			}

			return *this;
		}

		constexpr dynamic_array& operator=(std::initializer_list<value_type> ilist) {
			if (!empty())
				clear();

			_size = ilist.size();
			_data = _alloc_array(_size);

			auto it = ilist.begin();
			for (auto curr = _data; curr != _data + _size; ++curr, ++it)
				new (curr) value_type(*it);

			return *this;
		}

		/*
		*	Element Access
		*/

		constexpr reference at(size_type pos) {
			return _data[pos];
		}
		constexpr const_reference at(size_type pos) const {
			return _data[pos];
		}

		constexpr reference operator[](size_type pos) {
			return at(pos);
		}
		constexpr const_reference operator[](size_type pos) const {
			return at(pos);
		}

		constexpr reference front() {
			return at(0);
		}
		constexpr const_reference front() const {
			return at(0);
		}

		constexpr reference back() {
			return at(_size - 1);
		}
		constexpr const_reference back() const {
			return at(_size - 1);
		}

		constexpr pointer data() noexcept {
			return _data;
		}
		constexpr const_pointer data() const noexcept {
			return _data;
		}

		constexpr allocator_type get_allocator() const noexcept {
			return _alloc;
		}

		/*
		*	Iterators
		*/

		iterator begin() noexcept {
			return _data;
		}
		const_iterator begin() const noexcept {
			return _data;
		}
		const_iterator cbegin() const noexcept {
			return _data;
		}

		iterator end() noexcept {
			return _data + _size;
		}
		const_iterator end() const noexcept {
			return _data + _size;
		}
		const_iterator cend() const noexcept {
			return _data + _size;
		}

		reverse_iterator rbegin() noexcept {
			return end() - 1;
		}
		const_reverse_iterator rbegin() const noexcept {
			return end() - 1;
		}
		const_reverse_iterator crbegin() const noexcept {
			return cend() - 1;
		}

		reverse_iterator rend() noexcept {
			return begin() - 1;
		}
		const_reverse_iterator rend() const noexcept {
			return begin() - 1;
		}
		const_reverse_iterator crend() const noexcept {
			return cbegin() - 1;
		}

		/*
		*	Capacity
		*/

		constexpr bool empty() const noexcept {
			return _size == 0;
		}
		constexpr size_type size() const noexcept {
			return _size;
		}
		constexpr size_type max_size() const noexcept {
			return _size;
		}

		/*
		*	Modifiers
		*/

		constexpr void clear() noexcept {
			for (auto it = begin(); it != end(); ++it)
				it->~value_type();

			_dealloc_array(_data, _size);
			_data = nullptr, _size = 0;
		}

		constexpr void resize(size_type count) {
			if (count == 0) {
				if (!empty())
					clear();
			}
			else {
				auto newdata = _alloc_array(count);
				if (count > _size) {
					for (size_type i = 0; i < _size; i++)
						new (newdata + i) value_type(std::move(_data[i]));
					for (size_type i = _size; i < count; i++)
						new (newdata + i) value_type();
				}
				else {
					for (size_type i = 0; i < count; i++)
						new (newdata + i) value_type(std::move(_data[i]));
				}

				if (!empty())
					clear();
				_data = newdata, _size = count;
			}
		}
		constexpr void resize(size_type count, const value_type& value) {
			if (count == 0) {
				if (!empty())
					clear();
			}
			else {
				auto newdata = _alloc_array(count);
				if (count > _size) {
					for (size_type i = 0; i < _size; i++)
						new (newdata + i) value_type(std::move(_data[i]));
					for (size_type i = _size; i < count; i++)
						new (newdata + i) value_type(value);
				}
				else {
					for (size_type i = 0; i < count; i++)
						new (newdata + i) value_type(std::move(_data[i]));
				}

				if (!empty())
					clear();
				_data = newdata, _size = count;
			}
		}

	private:
		pointer _alloc_array(size_type size) {
			if (size == 0)
				return nullptr;
			return _alloc.allocate(size);
		}
		void _dealloc_array(pointer data, size_type size) {
			_alloc.deallocate(data, size);
		}

		allocator_type _alloc;

		size_type _size;
		pointer _data;
	};

}
