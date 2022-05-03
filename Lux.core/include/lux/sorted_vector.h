#pragma once

#include <lux/base_core.h>

#include <lux/types.h>
#include <lux/memory.h>
#include <lux/math.h>
#include <lux/functions.h>

#include <vector>

namespace lux
{

	template< class Key, class Value >
	class sorted_vector_type
	{
		template< class, class, class, class >
		friend class sorted_vector;

	public:
		using key_type = Key;
		using mapped_type = Value;

		using is_mapped = std::conjunction<
			std::negation<std::is_same<Value, void>>
		>;

		using value_type = std::conditional_t<is_mapped::value, std::pair<const key_type, mapped_type>, key_type>;

	protected:
		using _access_return_type = std::conditional_t<is_mapped::value, std::add_lvalue_reference_t<mapped_type>, key_type&>;
		using _const_access_return_type = std::conditional_t<is_mapped::value, std::add_lvalue_reference_t<const mapped_type>, const key_type&>;

		static inline const key_type& _key(const value_type& val) noexcept {
			if constexpr (is_mapped::value)
				return val.first;
			else
				return val;
		}

		static inline _access_return_type _value(value_type& val) noexcept {
			if constexpr (is_mapped::value)
				return val.second;
			else
				return val;
		}
		static inline _const_access_return_type _value(const value_type& val) noexcept {
			if constexpr (is_mapped::value)
				return val.second;
			else
				return val;
		}

	public:
		constexpr ~sorted_vector_type() = default;
		constexpr sorted_vector_type() 
			: _data() {
		}

		constexpr sorted_vector_type(sorted_vector_type & right) = default;
		constexpr sorted_vector_type(const sorted_vector_type & right) = default;

		constexpr sorted_vector_type(sorted_vector_type && right) = default;

	private:

	public:
		template< class... Args >
		constexpr sorted_vector_type(Args&&... args) 
			: _data(std::forward<Args>(args)...) {
			static_assert(std::is_constructible_v<value_type, Args...>, "value_type is not constructible with args");
		}

		inline const key_type& key() const noexcept {
			return _key(_data);
		}

		inline _access_return_type value() noexcept {
			return _value(_data);
		}
		inline _const_access_return_type value() const noexcept {
			return _value(_data);
		}

		inline value_type& data() noexcept {
			return _data;
		}
		inline const value_type& data() const noexcept {
			return _data;
		}

		template< class Arg >
		sorted_vector_type& operator=(Arg&& arg) {
			if constexpr (std::is_same_v<sorted_vector_type, std::decay_t<Arg>>)
				return *this = arg._data;
			else if constexpr (std::is_assignable_v<value_type, Arg>)
				_data = std::forward<Arg>(arg);
			else {
				static_assert(std::is_constructible_v<value_type, Arg>, "value_type is not constructible with Arg");

				auto addr = std::addressof(_data);
				std::destroy_at(addr);
				std::construct_at(addr, std::forward<Arg>(arg));
			}

			return *this;
		}

		sorted_vector_type& operator=(const sorted_vector_type& right) {
			return *this = right._data;
		}
		sorted_vector_type& operator=(sorted_vector_type&& right) noexcept(noexcept(*this = std::move(right._data))) {
			return *this = std::move(right._data);
		}

	private:
		value_type _data;
	};

	template<
		class Key, class Value = void,
		class Compare = std::less<Key>,
		class Alloc = std::allocator<sorted_vector_type<Key, Value>>
	>
	class sorted_vector
	{
		using _this_type = sorted_vector_type<Key, Value>;

	public:
		class value_compare;

		using key_type					= Key;
		using mapped_type				= Value;
		using value_type				= _this_type;
		using key_compare				= Compare;
		using value_compare				= sorted_vector::value_compare;
		using is_mapped					= _this_type::is_mapped;
		using vector_type				= std::vector<value_type, Alloc>;
		using allocator_type			= vector_type::allocator_type;
		using size_type					= vector_type::size_type;
		using difference_type			= vector_type::difference_type;
		using reference					= vector_type::reference;
		using const_reference			= vector_type::const_reference;
		using pointer					= vector_type::pointer;
		using const_pointer				= vector_type::const_pointer;
		using iterator					= vector_type::iterator;
		using const_iterator			= vector_type::const_iterator;
		using reverse_iterator			= vector_type::reverse_iterator;
		using const_reverse_iterator	= vector_type::const_reverse_iterator;

	private:
		template< class... Args >
		using _in_place_key_extractor = std::_In_place_key_extract_map<key_type, Args...>;

		template <class _Ty>
		using _remove_cvref_t = std::remove_cv_t<std::remove_reference_t<_Ty>>;

		using _access_return_type = _this_type::_access_return_type;
		using _const_access_return_type = _this_type::_const_access_return_type;

		// the container **must** have _key and _value

		static inline const key_type& _key(const value_type& value) {
			return value.key();
		}

		static inline _access_return_type _value(value_type& value) {
			return value.value();
		}
		static inline _const_access_return_type _value(const value_type& value) {
			return value.value();
		}

	public:
		static inline const key_type& get_key(const value_type& value) {
			return _key(value);
		}

		static inline _access_return_type get_value(value_type& value) {
			return _value(value);
		}
		static inline _const_access_return_type get_value(const value_type& value) {
			return _value(value);
		}

		constexpr ~sorted_vector() = default;

		constexpr sorted_vector(const key_compare& comp, const allocator_type& alloc = allocator_type())
			: _data(alloc), _comp(comp) {
		}

		constexpr sorted_vector()
			: sorted_vector(key_compare(), allocator_type()) {
		}
		explicit constexpr sorted_vector(const allocator_type& alloc)
			: sorted_vector(key_compare(), alloc) {
		}

		constexpr sorted_vector(const sorted_vector& other) = default;
		constexpr sorted_vector(const sorted_vector& other, const allocator_type& alloc)
			: _data(other._data, alloc), _comp(other._comp) {
		}

		constexpr sorted_vector(sorted_vector&& other) = default;
		constexpr sorted_vector(sorted_vector&& other, const allocator_type& alloc)
			: _data(std::move(other._data), alloc), _comp(std::move(other._comp)) {
		}

		constexpr sorted_vector& operator=(const sorted_vector& other) = default;
		constexpr sorted_vector& operator=(sorted_vector&& other) = default;

		constexpr vector_type& vector() noexcept {
			return _data;
		}
		constexpr const vector_type& vector() const noexcept {
			return _data;
		}

		constexpr allocator_type get_allocator() const noexcept {
			return _data.get_allocator();
		}

		class value_compare
		{
			friend class sorted_vector;
		public:
			constexpr ~value_compare() = default;
		protected:
			constexpr value_compare(sorted_vector::key_compare comp) : _comp(comp) { }

		public:
			inline bool operator()(const sorted_vector::value_type& left, const sorted_vector::value_type& right) noexcept(noexcept(_comp(_key(left), _key(right)))) {
				return _comp(_key(left), _key(right));
			}

		protected:
			sorted_vector::key_compare _comp;
		};

		constexpr key_compare key_comp() const {
			return _comp;
		}
		constexpr value_compare value_comp() const {
			return value_compare{ _comp };
		}

		constexpr bool empty() const noexcept {
			return _data.empty();
		}
		constexpr size_type size() const noexcept {
			return _data.size();
		}
		constexpr size_type max_size() const noexcept {
			return _data.max_size();
		}

		constexpr iterator begin() noexcept {
			return _data.begin();
		}
		constexpr const_iterator begin() const noexcept {
			return _data.begin();
		}
		constexpr const_iterator cbegin() const noexcept {
			return _data.cbegin();
		}

		constexpr iterator end() noexcept {
			return _data.end();
		}
		constexpr const_iterator end() const noexcept {
			return _data.end();
		}
		constexpr const_iterator cend() const noexcept {
			return _data.cend();
		}

		constexpr reverse_iterator rbegin() noexcept {
			return _data.rbegin();
		}
		constexpr const_reverse_iterator rbegin() const noexcept {
			return _data.rbegin();
		}
		constexpr const_reverse_iterator crbegin() const noexcept {
			return _data.crbegin();
		}

		constexpr reverse_iterator rend() noexcept {
			return _data.rend();
		}
		constexpr const_reverse_iterator rend() const noexcept {
			return _data.rend();
		}
		constexpr const_reverse_iterator crend() const noexcept {
			return _data.crend();
		}

		constexpr void clear() noexcept {
			_data.clear();
		}

	private:
		constexpr size_type _lower_bound(const key_type& key) const {
			size_type mid = 0, left = 0, right = size();
			while (left < right) {
				mid = (left + right) / 2;
				if (_comp(key, _key(_data.at(mid))))
					right = mid;
				else if (_comp(_key(_data.at(mid)), key))
					left = mid + 1;
				else
					return mid;
			}

			return left;
		}

		constexpr bool _lower_bound_match(const key_type& res, const key_type& key) const {
			// lb must be less or equal to key
			return !_comp(key, res);
		}
		constexpr bool _lower_bound_match(size_type pos, const key_type& key) const {
			// lb must be less or equal to key
			return pos < size() && _lower_bound_match(_key(_data.at(pos)), key);
		}

		template< class... Args >
		constexpr std::pair<iterator, bool> _emplace(Args&&... args) {
			using _key_extractor = _in_place_key_extractor<_remove_cvref_t<Args>...>;

			if constexpr (is_mapped::value && _key_extractor::_Extractable) {
				const auto& key	= _key_extractor::_Extract(args...);
				auto pos		= _lower_bound(key);
				if (_lower_bound_match(pos, key))
					return { begin() + pos, false };
				return { _data.emplace(begin() + pos, std::forward<Args>(args)...), true };
			}
			else {
				value_type val{ std::forward<Args>(args)... };
				const auto& key	= _key(val);
				auto pos		= _lower_bound(key);
				if (_lower_bound_match(pos, key))
					return { begin() + pos, false };
				return { _data.emplace(begin() + pos, std::move(val)), true };
			}
		}

	public:
		template< class... Args >
		constexpr std::pair<iterator, bool> emplace(Args&&... args) {
			return _emplace(std::forward<Args>(args)...);
		}

		constexpr std::pair<iterator, bool> insert(const value_type& value) {
			return _emplace(value);
		}
		constexpr std::pair<iterator, bool> insert(value_type&& value) {
			return _emplace(value);
		}
		template< class P, std::enable_if_t<std::is_constructible_v<value_type, P>, int> = 0>
		constexpr std::pair<iterator, bool> insert(P&& value) {
			return _emplace(std::forward<P>(value));
		}

		template< class It >
		constexpr void insert(It first, It last) {
			using deref_type = lux::deref_t<It>;
			_data.reserve(_data.size() + std::distance(first, last));

			auto it			= (first);
			const auto end	= (last);
			for (; it != end; ++it)
				_emplace(*it);
		}
		constexpr void insert(std::initializer_list<value_type> ilist) {
			insert(ilist.begin(), ilist.end());
		}

		template< class It >
		constexpr sorted_vector(It first, It last, const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type())
			: sorted_vector(comp, alloc) {
			insert(first, last);
		}
		template< class It >
		constexpr sorted_vector(It first, It last, const allocator_type& alloc)
			: sorted_vector(first, last, key_compare(), alloc) {
		}

		constexpr sorted_vector(std::initializer_list<value_type> ilist, const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type())
			: sorted_vector(comp, alloc) {
			insert(ilist);
		}
		constexpr sorted_vector(std::initializer_list<value_type> ilist, const allocator_type& alloc)
			: sorted_vector(ilist, key_compare(), alloc) {
		}

		constexpr sorted_vector& operator=(std::initializer_list<value_type> ilist) {
			clear();
			insert(ilist);

			return *this;
		}

	private:
		template< class K, class... Args >
		constexpr std::pair<iterator, bool> _try_emplace(K&& k, Args&&... args) {
			static_assert(is_mapped::value, "mapped_value must be valid");

			auto pos = _lower_bound(k);
			if (_lower_bound_match(pos, k))
				return { begin() + pos, false };

			return { _data.emplace(begin() + pos, value_type(std::forward<K>(k), mapped_type(std::forward<Args>(args)...))), true };
		}

	public:
		template< class... Args >
		constexpr std::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args) {
			return _try_emplace(k, std::forward<Args>(args)...);
		}
		template< class... Args >
		constexpr std::pair<iterator, bool> try_emplace(key_type&& k, Args&&... args) {
			return _try_emplace(std::move(k), std::forward<Args>(args)...);
		}

	private:
		template< class K, class T >
		constexpr std::pair<iterator, bool> _insert_or_assign(K&& key, T&& val) {
			static_assert(is_mapped::value, "mapped_value must be valid");

			auto pos = _lower_bound(key);
			if (_lower_bound_match(pos, key)) {
				_value(_data.at(pos)) = std::forward<T>(val);
				return { begin() + pos, false };
			}

			return _emplace(std::forward<K>(key), std::forward<T>(val));
		}

	public:
		template< class T >
		constexpr std::pair<iterator, bool> insert_or_assign(const key_type& key, T&& val) {
			return _insert_or_assign(key, std::forward<T>(val));
		}
		template< class T >
		constexpr std::pair<iterator, bool> insert_or_assign(key_type&& key, T&& val) {
			return _insert_or_assign(std::move(key), std::forward<T>(val));
		}

		constexpr iterator erase(const_iterator it) {
			return _data.erase(it);
		}
		constexpr iterator erase(const_iterator first, const_iterator last) {
			return _data.erase(first, last);
		}

	private:
		constexpr size_type _erase(const key_type& key) {
			auto pos = _lower_bound(key);
			if (!_lower_bound_match(pos, key))
				return 0;
			erase(cbegin() + pos);
			return 1;
		}

	public:
		constexpr size_type erase(const key_type& key) {
			return _erase(key);
		}

		constexpr void swap(sorted_vector&& other) noexcept {
			_data.swap(other._data);
		}

	public:
		constexpr _access_return_type at(const key_type& key) {
			auto pos	= _lower_bound(key);
			auto& val	= _data.at(pos);
			if (_lower_bound_match(_key(val), key))
				return _value(val);

			if constexpr (is_mapped::value)
				throw std::out_of_range("invalid lux::sorted_vector<K, T> key");
			else
				throw std::out_of_range("invalid lux::sorted_vector<K> key");
		}
		constexpr _const_access_return_type at(const key_type& key) const {
			auto pos	= _lower_bound(key);
			auto& val	= _data.at(pos);
			if (_lower_bound_match(_key(val), key))
				return _value(val);

			if constexpr (is_mapped::value)
				throw std::out_of_range("invalid lux::sorted_vector<K, T> key");
			else
				throw std::out_of_range("invalid lux::sorted_vector<K> key");
		}

	private:
		template< class K >
		constexpr _access_return_type _op_array(K&& key) {
			auto pos	= _lower_bound(key);
			auto& val	= _data.at(pos);
			if (_lower_bound_match(_key(val), key))
				return _value(val);

			if constexpr (is_mapped::value)
				return _value(*_emplace(std::forward<K>(key), mapped_type{}).first);
			else
				return _value(*_emplace(std::forward<K>(key)).first);
		}

	public:
		constexpr _access_return_type operator[](const key_type& key) {
			return _op_array(key);
		}
		constexpr _access_return_type operator[](key_type&& key) {
			return _op_array(std::move(key));
		}

		constexpr size_type count(const key_type& key) const noexcept {
			if (_lower_bound_match(_lower_bound(key), key))
				return 1;
			return 0;
		}

		constexpr bool contains(const key_type& key) const noexcept {
			return _lower_bound_match(_lower_bound(key), key);
		}

	private:
		constexpr size_type _find(const key_type& key) const {
			auto pos	= _lower_bound(key);
			auto len	= size();

			if (pos >= len)
				return len;
			if (_lower_bound_match(_key(_data.at(pos)), key))
				return pos;
			return len;
		}

	public:
		constexpr iterator find(const key_type& key) {
			return begin() + _find(key);
		}
		constexpr const_iterator find(const key_type& key) const {
			return begin() + _find(key);
		}

		constexpr iterator lower_bound(const key_type& key) {
			return begin() + _lower_bound(key);
		}
		constexpr const_iterator lower_bound(const key_type& key) const {
			return begin() + _lower_bound(key);
		}

	private:
		constexpr size_type _upper_bound(const key_type& key) const {
			size_type mid = 0, left = 0, right = size();
			while (left < right) {
				mid = (left + right) / 2;
				if (_comp(key, _key(_data.at(mid))))
					right = mid;
				else
					left = mid + 1;
			}

			return left;
		}

	public:
		constexpr iterator upper_bound(const key_type& key) {
			return begin() + _upper_bound(key);
		}
		constexpr const_iterator upper_bound(const key_type& key) const {
			return begin() + _upper_bound(key);
		}

		constexpr std::pair<iterator, iterator> equal_range(const key_type& key) {
			return { lower_bound(key), upper_bound(key) };
		}
		constexpr std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const {
			return { lower_bound(key), upper_bound(key) };
		}

	private:

		vector_type _data;
		key_compare _comp;
	};

}
