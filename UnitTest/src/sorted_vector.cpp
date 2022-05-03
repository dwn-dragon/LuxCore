#include "head.h"

namespace lux::test::containers
{

	constexpr bool compare_cmplx(const complex_t& left, const complex_t& right) {
		if (left.y < right.y || (left.y == right.y && left.x < left.x))
			return true;
		return false;

		ToString(true);
	}

	constexpr int MAX_KEY_VALUE = 10000;
	constexpr int MAX_KEY_X_VALUE = MAX_KEY_VALUE;
	constexpr int MAX_KEY_Y_VALUE = MAX_KEY_VALUE;

	constexpr int MAX_MAPPED_VALUE = std::numeric_limits<int>::max();
	constexpr int MAX_MAPPED_X_VALUE = MAX_MAPPED_VALUE;
	constexpr int MAX_MAPPED_Y_VALUE = MAX_MAPPED_VALUE;

	constexpr size_t KEYC = 500;

	template< class T >
	T::key_type generate_key() {
		if constexpr (std::is_same_v<typename T::key_type, simple_t>)
			return rand() % MAX_KEY_VALUE;
		else if constexpr (std::is_same_v<typename T::key_type, complex_t>)
			return { rand() % MAX_KEY_X_VALUE, rand() % MAX_KEY_Y_VALUE };
	}
	template< class T >
	std::conditional_t<T::is_mapped::value, typename T::mapped_type, typename T::key_type> generate_mapped() {
		if constexpr (std::is_same_v<typename T::mapped_type, simple_t>)
			return rand() % MAX_MAPPED_VALUE;
		else if constexpr (std::is_same_v<typename T::mapped_type, complex_t>)
			return { rand() % MAX_MAPPED_X_VALUE, rand() % MAX_MAPPED_Y_VALUE };
		else if constexpr (!T::is_mapped::value)
			return T::key_type();
	}

	template< class T >
	T::value_type generate_value() {
		if constexpr (T::is_mapped::value) {
			return { generate_key<T>(), generate_value<T>() };
		}
		else {
			return { generate_value<T>() };
		}
	}

	template< class Ty >
	void generate(Ty& ov, std::vector<typename Ty::key_type>& keys, std::vector<typename Ty::value_type>& values) {
		for (size_t i = 0; i < KEYC; i++) {
			auto key = generate_key<Ty>();
			if constexpr (Ty::is_mapped::value) {
				auto res = ov.emplace(key, generate_mapped<Ty>());
				if (res.second) {
					keys.push_back(key);
					values.push_back(*res.first);
				}
			}
			else {
				auto res = ov.emplace(key);
				if (res.second) {
					keys.push_back(key);
					values.push_back(*res.first);
				}
			}
		}
	}
	template< bool Multi, class K, class T >
	bool sorted(const lux::sorted_vector<K, T>& storage) {
		auto comp = storage.value_comp();
		for (auto curr = storage.begin(), prev = curr++; curr != storage.end(); prev = curr++) {
			if constexpr (Multi) {
				// allows duplicates of same value
				// !(prev < curr) 
				// -> prev >= curr
				if (!comp(*prev, *curr))
					return false;
			}
			else {
				// doesn't allow duplicates of same value
				// curr < prev
				// -> prev > curr
				if (comp(*curr, *prev))
					return false;
			}
		}

		return true;
	}

	template< class Ty >
	void integrity_check(Ty& ov, std::vector<typename Ty::value_type>& values) {
		Assert::AreEqual(values.size(), ov.size(), L"integrity check failed: sizes mismatch");
		Assert::IsTrue(sorted<true>(ov), L"integrity check failed: sorted<true> failed");
		Assert::IsTrue(sorted<false>(ov)), L"integrity check failed: sorted<false> failed";

		for (auto& el : values) {
			auto res = ov.find(el.key());
			if (res == ov.end())
				Assert::Fail(L"integrity check failed: missing element");
			if (res->key() != el.key())
				Assert::Fail(L"integrity check failed: keys mismatch");
			if (res->value() != el.value())
				Assert::Fail(L"integrity check failed: keys mismatch");
		}
	}

	TEST_CLASS(sorted_vector_simple_none)
	{
		using ov_type = lux::sorted_vector<simple_t, void>;

		using keys_type = std::vector<ov_type::key_type>;
		using values_type = std::vector<ov_type::value_type>;

		ov_type _ov;

		keys_type _keys;
		values_type _values;

	public:
		sorted_vector_simple_none() {
			srand(time(nullptr));
			generate(_ov, _keys, _values);
		}

		TEST_METHOD(multis_ordered) {
			Assert::IsTrue(sorted<true>(_ov));
		}
		TEST_METHOD(no_multi_ordered) {
			Assert::IsTrue(sorted<false>(_ov));
		}

		TEST_METHOD(itrs_constructor) {
			ov_type ov(_values.begin(), _values.end());
			integrity_check(ov, _values);
		}

		template< class T = void >
		TEST_METHOD(insert) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					ov_type::value_type v{ key, generate_mapped<ov_type>() };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					ov_type::value_type v{ key };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}
		TEST_METHOD(emplace) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					auto res = ov.emplace(key, generate_mapped<ov_type>());

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					auto res = ov.emplace(key);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}

		TEST_METHOD(try_emplace) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.try_emplace(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"try_emplace failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(insert_or_assign) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.insert_or_assign(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"insert_or_assign failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
					else for (auto& el : _values) {
						if (el.key() == key)
							el.value() = val;
					}
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto pos = rand() % _keys.size();
				auto& key = _keys[pos];

				auto res = _ov.erase(key);
				Assert::IsTrue(res, L"erase failure");

				_keys.erase(_keys.begin() + pos);
				_values.erase(_values.begin() + pos);
			}
		}
		TEST_METHOD(random_key_erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto key = generate_key<ov_type>();
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::IsFalse(_ov.erase(key), L"erase mismatch results");
				else try {
					auto res = _ov.erase(key);
					Assert::IsTrue(res, L"erase failure");

					for (auto k = _keys.begin(); k != _keys.end(); ++k) {
						if (*k == key) {
							_keys.erase(k);
							break;
						}
					}
					for (auto v = _values.begin(); v != _values.end(); ++v) {
						if (v->key() == key) {
							_values.erase(v);
							break;
						}
					}
				}
				catch (const std::exception& e) {
					std::wstringstream wss;
					wss << e.what();
					auto err = wss.str();

					Assert::Fail(err.c_str());
				}
			}
		}

		TEST_METHOD(empty) {
			ov_type ov;
			ov = _ov;

			ov.clear();
			Assert::IsTrue(ov.empty());
		}
		TEST_METHOD(size) {
			Assert::AreEqual(_ov.size(), _values.size());
		}

		TEST_METHOD(at) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov.at(key);

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(op_array) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov[key];

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(find) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");

				Assert::AreEqual(_ov.get_key(_values[i]), _ov.get_key(*it), L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), _ov.get_value(*it), L"values mismatch");
			}
		}
		TEST_METHOD(cross_search) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];

				auto& val = _ov.at(key);
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");
				Assert::AreEqual(_ov.get_key(*it), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(*it), val, L"values mismatch");
			}
		}

		TEST_METHOD(lower_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.lower_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) >= key))
					Assert::Fail();
			}
		}
		TEST_METHOD(upper_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.upper_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) > key))
					Assert::Fail();
			}
		}
		TEST_METHOD(equal_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto eb = _ov.equal_range(key);

				for (auto it = eb.first; it != eb.second; ++it) {
					if (it != _ov.end() && !(_ov.get_key(*it) == key))
						Assert::Fail();
				}
			}
		}

	};

	TEST_CLASS(sorted_vector_simple_simple)
	{
		using ov_type = lux::sorted_vector<simple_t, simple_t>;

		using keys_type = std::vector<ov_type::key_type>;
		using values_type = std::vector<ov_type::value_type>;

		ov_type _ov;

		keys_type _keys;
		values_type _values;

	public:
		sorted_vector_simple_simple() {
			srand(time(nullptr));
			generate(_ov, _keys, _values);
		}

		TEST_METHOD(multis_ordered) {
			Assert::IsTrue(sorted<true>(_ov));
		}
		TEST_METHOD(no_multi_ordered) {
			Assert::IsTrue(sorted<false>(_ov));
		}

		TEST_METHOD(itrs_constructor) {
			ov_type ov(_values.begin(), _values.end());
			integrity_check(ov, _values);
		}

		template< class T = void >
		TEST_METHOD(insert) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					ov_type::value_type v{ key, generate_mapped<ov_type>() };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					ov_type::value_type v{ key };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}
		TEST_METHOD(emplace) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					auto res = ov.emplace(key, generate_mapped<ov_type>());

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					auto res = ov.emplace(key);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}

		TEST_METHOD(try_emplace) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.try_emplace(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"try_emplace failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(insert_or_assign) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.insert_or_assign(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"insert_or_assign failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
					else for (auto& el : _values) {
						if (el.key() == key)
							el.value() = val;
					}
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto pos = rand() % _keys.size();
				auto& key = _keys[pos];

				auto res = _ov.erase(key);
				Assert::IsTrue(res, L"erase failure");

				_keys.erase(_keys.begin() + pos);
				_values.erase(_values.begin() + pos);
			}
		}
		TEST_METHOD(random_key_erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto key = generate_key<ov_type>();
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::IsFalse(_ov.erase(key), L"erase mismatch results");
				else try {
					auto res = _ov.erase(key);
					Assert::IsTrue(res, L"erase failure");

					for (auto k = _keys.begin(); k != _keys.end(); ++k) {
						if (*k == key) {
							_keys.erase(k);
							break;
						}
					}
					for (auto v = _values.begin(); v != _values.end(); ++v) {
						if (v->key() == key) {
							_values.erase(v);
							break;
						}
					}
				}
				catch (const std::exception& e) {
					std::wstringstream wss;
					wss << e.what();
					auto err = wss.str();

					Assert::Fail(err.c_str());
				}
			}
		}

		TEST_METHOD(empty) {
			ov_type ov;
			ov = _ov;

			ov.clear();
			Assert::IsTrue(ov.empty());
		}
		TEST_METHOD(size) {
			Assert::AreEqual(_ov.size(), _values.size());
		}

		TEST_METHOD(at) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov.at(key);

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(op_array) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov[key];

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(find) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");

				Assert::AreEqual(_ov.get_key(_values[i]), _ov.get_key(*it), L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), _ov.get_value(*it), L"values mismatch");
			}
		}
		TEST_METHOD(cross_search) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];

				auto& val = _ov.at(key);
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");
				Assert::AreEqual(_ov.get_key(*it), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(*it), val, L"values mismatch");
			}
		}

		TEST_METHOD(lower_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.lower_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) >= key))
					Assert::Fail();
			}
		}
		TEST_METHOD(upper_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.upper_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) > key))
					Assert::Fail();
			}
		}
		TEST_METHOD(equal_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto eb = _ov.equal_range(key);

				for (auto it = eb.first; it != eb.second; ++it) {
					if (it != _ov.end() && !(_ov.get_key(*it) == key))
						Assert::Fail();
				}
			}
		}

	};

	TEST_CLASS(sorted_vector_simple_complex)
	{
		using ov_type = lux::sorted_vector<simple_t, complex_t>;

		using keys_type = std::vector<ov_type::key_type>;
		using values_type = std::vector<ov_type::value_type>;

		ov_type _ov;

		keys_type _keys;
		values_type _values;

	public:
		sorted_vector_simple_complex() {
			srand(time(nullptr));
			generate(_ov, _keys, _values);
		}

		TEST_METHOD(multis_ordered) {
			Assert::IsTrue(sorted<true>(_ov));
		}
		TEST_METHOD(no_multi_ordered) {
			Assert::IsTrue(sorted<false>(_ov));
		}

		TEST_METHOD(itrs_constructor) {
			ov_type ov(_values.begin(), _values.end());
			integrity_check(ov, _values);
		}

		template< class T = void >
		TEST_METHOD(insert) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					ov_type::value_type v{ key, generate_mapped<ov_type>() };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					ov_type::value_type v{ key };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}
		TEST_METHOD(emplace) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					auto res = ov.emplace(key, generate_mapped<ov_type>());

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					auto res = ov.emplace(key);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}

		TEST_METHOD(try_emplace) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.try_emplace(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"try_emplace failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(insert_or_assign) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.insert_or_assign(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"insert_or_assign failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
					else for (auto& el : _values) {
						if (el.key() == key)
							el.value() = val;
					}
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto pos = rand() % _keys.size();
				auto& key = _keys[pos];

				auto res = _ov.erase(key);
				Assert::IsTrue(res, L"erase failure");

				_keys.erase(_keys.begin() + pos);
				_values.erase(_values.begin() + pos);
			}
		}
		TEST_METHOD(random_key_erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto key = generate_key<ov_type>();
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::IsFalse(_ov.erase(key), L"erase mismatch results");
				else try {
					auto res = _ov.erase(key);
					Assert::IsTrue(res, L"erase failure");

					for (auto k = _keys.begin(); k != _keys.end(); ++k) {
						if (*k == key) {
							_keys.erase(k);
							break;
						}
					}
					for (auto v = _values.begin(); v != _values.end(); ++v) {
						if (v->key() == key) {
							_values.erase(v);
							break;
						}
					}
				}
				catch (const std::exception& e) {
					std::wstringstream wss;
					wss << e.what();
					auto err = wss.str();

					Assert::Fail(err.c_str());
				}
			}
		}

		TEST_METHOD(empty) {
			ov_type ov;
			ov = _ov;

			ov.clear();
			Assert::IsTrue(ov.empty());
		}
		TEST_METHOD(size) {
			Assert::AreEqual(_ov.size(), _values.size());
		}

		TEST_METHOD(at) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov.at(key);

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(op_array) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov[key];

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(find) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");

				Assert::AreEqual(_ov.get_key(_values[i]), _ov.get_key(*it), L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), _ov.get_value(*it), L"values mismatch");
			}
		}
		TEST_METHOD(cross_search) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];

				auto& val = _ov.at(key);
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");
				Assert::AreEqual(_ov.get_key(*it), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(*it), val, L"values mismatch");
			}
		}

		TEST_METHOD(lower_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.lower_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) >= key))
					Assert::Fail();
			}
		}
		TEST_METHOD(upper_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.upper_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) > key))
					Assert::Fail();
			}
		}
		TEST_METHOD(equal_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto eb = _ov.equal_range(key);

				for (auto it = eb.first; it != eb.second; ++it) {
					if (it != _ov.end() && !(_ov.get_key(*it) == key))
						Assert::Fail();
				}
			}
		}

	};

	TEST_CLASS(sorted_vector_complex_none)
	{
		using ov_type = lux::sorted_vector<complex_t, void>;

		using keys_type = std::vector<ov_type::key_type>;
		using values_type = std::vector<ov_type::value_type>;

		ov_type _ov;

		keys_type _keys;
		values_type _values;

	public:
		sorted_vector_complex_none() {
			srand(time(nullptr));
			generate(_ov, _keys, _values);
		}

		TEST_METHOD(multis_ordered) {
			Assert::IsTrue(sorted<true>(_ov));
		}
		TEST_METHOD(no_multi_ordered) {
			Assert::IsTrue(sorted<false>(_ov));
		}

		TEST_METHOD(itrs_constructor) {
			ov_type ov(_values.begin(), _values.end());
			integrity_check(ov, _values);
		}

		template< class T = void >
		TEST_METHOD(insert) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					ov_type::value_type v{ key, generate_mapped<ov_type>() };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					ov_type::value_type v{ key };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}
		TEST_METHOD(emplace) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					auto res = ov.emplace(key, generate_mapped<ov_type>());

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					auto res = ov.emplace(key);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}

		TEST_METHOD(try_emplace) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.try_emplace(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"try_emplace failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(insert_or_assign) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.insert_or_assign(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"insert_or_assign failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
					else for (auto& el : _values) {
						if (el.key() == key)
							el.value() = val;
					}
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto pos = rand() % _keys.size();
				auto& key = _keys[pos];

				auto res = _ov.erase(key);
				Assert::IsTrue(res, L"erase failure");

				_keys.erase(_keys.begin() + pos);
				_values.erase(_values.begin() + pos);
			}
		}
		TEST_METHOD(random_key_erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto key = generate_key<ov_type>();
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::IsFalse(_ov.erase(key), L"erase mismatch results");
				else try {
					auto res = _ov.erase(key);
					Assert::IsTrue(res, L"erase failure");

					for (auto k = _keys.begin(); k != _keys.end(); ++k) {
						if (*k == key) {
							_keys.erase(k);
							break;
						}
					}
					for (auto v = _values.begin(); v != _values.end(); ++v) {
						if (v->key() == key) {
							_values.erase(v);
							break;
						}
					}
				}
				catch (const std::exception& e) {
					std::wstringstream wss;
					wss << e.what();
					auto err = wss.str();

					Assert::Fail(err.c_str());
				}
			}
		}

		TEST_METHOD(empty) {
			ov_type ov;
			ov = _ov;

			ov.clear();
			Assert::IsTrue(ov.empty());
		}
		TEST_METHOD(size) {
			Assert::AreEqual(_ov.size(), _values.size());
		}

		TEST_METHOD(at) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov.at(key);

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(op_array) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov[key];

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(find) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");

				Assert::AreEqual(_ov.get_key(_values[i]), _ov.get_key(*it), L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), _ov.get_value(*it), L"values mismatch");
			}
		}
		TEST_METHOD(cross_search) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];

				auto& val = _ov.at(key);
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");
				Assert::AreEqual(_ov.get_key(*it), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(*it), val, L"values mismatch");
			}
		}

		TEST_METHOD(lower_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.lower_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) >= key))
					Assert::Fail();
			}
		}
		TEST_METHOD(upper_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.upper_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) > key))
					Assert::Fail();
			}
		}
		TEST_METHOD(equal_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto eb = _ov.equal_range(key);

				for (auto it = eb.first; it != eb.second; ++it) {
					if (it != _ov.end() && !(_ov.get_key(*it) == key))
						Assert::Fail();
				}
			}
		}

	};

	TEST_CLASS(sorted_vector_complex_simple)
	{
		using ov_type = lux::sorted_vector<complex_t, simple_t>;

		using keys_type = std::vector<ov_type::key_type>;
		using values_type = std::vector<ov_type::value_type>;

		ov_type _ov;

		keys_type _keys;
		values_type _values;

	public:
		sorted_vector_complex_simple() {
			srand(time(nullptr));
			generate(_ov, _keys, _values);
		}

		TEST_METHOD(multis_ordered) {
			Assert::IsTrue(sorted<true>(_ov));
		}
		TEST_METHOD(no_multi_ordered) {
			Assert::IsTrue(sorted<false>(_ov));
		}

		TEST_METHOD(itrs_constructor) {
			ov_type ov(_values.begin(), _values.end());
			integrity_check(ov, _values);
		}

		template< class T = void >
		TEST_METHOD(insert) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					ov_type::value_type v{ key, generate_mapped<ov_type>() };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					ov_type::value_type v{ key };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}
		TEST_METHOD(emplace) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					auto res = ov.emplace(key, generate_mapped<ov_type>());

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					auto res = ov.emplace(key);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}

		TEST_METHOD(try_emplace) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.try_emplace(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"try_emplace failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(insert_or_assign) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.insert_or_assign(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"insert_or_assign failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
					else for (auto& el : _values) {
						if (el.key() == key)
							el.value() = val;
					}
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto pos = rand() % _keys.size();
				auto& key = _keys[pos];

				auto res = _ov.erase(key);
				Assert::IsTrue(res, L"erase failure");

				_keys.erase(_keys.begin() + pos);
				_values.erase(_values.begin() + pos);
			}
		}
		TEST_METHOD(random_key_erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto key = generate_key<ov_type>();
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::IsFalse(_ov.erase(key), L"erase mismatch results");
				else try {
					auto res = _ov.erase(key);
					Assert::IsTrue(res, L"erase failure");

					for (auto k = _keys.begin(); k != _keys.end(); ++k) {
						if (*k == key) {
							_keys.erase(k);
							break;
						}
					}
					for (auto v = _values.begin(); v != _values.end(); ++v) {
						if (v->key() == key) {
							_values.erase(v);
							break;
						}
					}
				}
				catch (const std::exception& e) {
					std::wstringstream wss;
					wss << e.what();
					auto err = wss.str();

					Assert::Fail(err.c_str());
				}
			}
		}

		TEST_METHOD(empty) {
			ov_type ov;
			ov = _ov;

			ov.clear();
			Assert::IsTrue(ov.empty());
		}
		TEST_METHOD(size) {
			Assert::AreEqual(_ov.size(), _values.size());
		}

		TEST_METHOD(at) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov.at(key);

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(op_array) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov[key];

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(find) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");

				Assert::AreEqual(_ov.get_key(_values[i]), _ov.get_key(*it), L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), _ov.get_value(*it), L"values mismatch");
			}
		}
		TEST_METHOD(cross_search) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];

				auto& val = _ov.at(key);
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");
				Assert::AreEqual(_ov.get_key(*it), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(*it), val, L"values mismatch");
			}
		}

		TEST_METHOD(lower_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.lower_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) >= key))
					Assert::Fail();
			}
		}
		TEST_METHOD(upper_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.upper_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) > key))
					Assert::Fail();
			}
		}
		TEST_METHOD(equal_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto eb = _ov.equal_range(key);

				for (auto it = eb.first; it != eb.second; ++it) {
					if (it != _ov.end() && !(_ov.get_key(*it) == key))
						Assert::Fail();
				}
			}
		}

	};

	TEST_CLASS(sorted_vector_complex_complex)
	{
		using ov_type = lux::sorted_vector<complex_t, complex_t>;

		using keys_type = std::vector<ov_type::key_type>;
		using values_type = std::vector<ov_type::value_type>;

		ov_type _ov;

		keys_type _keys;
		values_type _values;

	public:
		sorted_vector_complex_complex() {
			srand(time(nullptr));
			generate(_ov, _keys, _values);
		}

		TEST_METHOD(multis_ordered) {
			Assert::IsTrue(sorted<true>(_ov));
		}
		TEST_METHOD(no_multi_ordered) {
			Assert::IsTrue(sorted<false>(_ov));
		}

		TEST_METHOD(itrs_constructor) {
			ov_type ov(_values.begin(), _values.end());
			integrity_check(ov, _values);
		}

		template< class T = void >
		TEST_METHOD(insert) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					ov_type::value_type v{ key, generate_mapped<ov_type>() };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					ov_type::value_type v{ key };
					auto res = ov.insert(v);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}
		TEST_METHOD(emplace) {
			ov_type ov;
			size_t ks = 0;

			for (size_t i = 0; i < KEYC; i++) {
				auto key = generate_key<ov_type>();
				if constexpr (ov_type::is_mapped::value) {
					auto res = ov.emplace(key, generate_mapped<ov_type>());

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
				else {
					auto res = ov.emplace(key);

					if (ov.find(key) == ov.end())
						Assert::Fail(L"element not found");
					if (res.second)
						++ks;
				}
			}

			Assert::AreEqual(ks, ov.size(), L"sizes mismatch");
			Assert::IsTrue(sorted<true>(ov), L"sorted<true> failed");
			Assert::IsTrue(sorted<false>(ov)), L"sorted<false> failed";
		}

		TEST_METHOD(try_emplace) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.try_emplace(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"try_emplace failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(insert_or_assign) {
			if constexpr (ov_type::is_mapped::value) {
				for (size_t i = 0; i < 100; i++) {
					auto key = generate_key<ov_type>();
					auto val = generate_mapped<ov_type>();

					auto fres = _ov.find(key) == _ov.end();
					auto tres = _ov.insert_or_assign(key, val);

					if (fres != tres.second || _ov.find(key) == _ov.end()) {
						Assert::Fail(L"insert_or_assign failure");
					}

					if (tres.second)
						_values.push_back(*tres.first);
					else for (auto& el : _values) {
						if (el.key() == key)
							el.value() = val;
					}
				}

				integrity_check(_ov, _values);
			}
		}

		TEST_METHOD(erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto pos = rand() % _keys.size();
				auto& key = _keys[pos];

				auto res = _ov.erase(key);
				Assert::IsTrue(res, L"erase failure");

				_keys.erase(_keys.begin() + pos);
				_values.erase(_values.begin() + pos);
			}
		}
		TEST_METHOD(random_key_erase) {
			constexpr size_t TO_ERASE = 10;
			for (size_t i = 0; i < TO_ERASE; i++) {
				auto key = generate_key<ov_type>();
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::IsFalse(_ov.erase(key), L"erase mismatch results");
				else try {
					auto res = _ov.erase(key);
					Assert::IsTrue(res, L"erase failure");

					for (auto k = _keys.begin(); k != _keys.end(); ++k) {
						if (*k == key) {
							_keys.erase(k);
							break;
						}
					}
					for (auto v = _values.begin(); v != _values.end(); ++v) {
						if (v->key() == key) {
							_values.erase(v);
							break;
						}
					}
				}
				catch (const std::exception& e) {
					std::wstringstream wss;
					wss << e.what();
					auto err = wss.str();

					Assert::Fail(err.c_str());
				}
			}
		}

		TEST_METHOD(empty) {
			ov_type ov;
			ov = _ov;

			ov.clear();
			Assert::IsTrue(ov.empty());
		}
		TEST_METHOD(size) {
			Assert::AreEqual(_ov.size(), _values.size());
		}

		TEST_METHOD(at) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov.at(key);

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(op_array) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto& val = _ov[key];

				Assert::AreEqual(_ov.get_key(_values[i]), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), val, L"values mismatch");
			}
		}
		TEST_METHOD(find) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");

				Assert::AreEqual(_ov.get_key(_values[i]), _ov.get_key(*it), L"keys mismatch");
				Assert::AreEqual(_ov.get_value(_values[i]), _ov.get_value(*it), L"values mismatch");
			}
		}
		TEST_METHOD(cross_search) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];

				auto& val = _ov.at(key);
				auto it = _ov.find(key);

				if (it == _ov.end())
					Assert::Fail(L"key not found");
				Assert::AreEqual(_ov.get_key(*it), key, L"keys mismatch");
				Assert::AreEqual(_ov.get_value(*it), val, L"values mismatch");
			}
		}

		TEST_METHOD(lower_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.lower_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) >= key))
					Assert::Fail();
			}
		}
		TEST_METHOD(upper_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto res = _ov.upper_bound(key);

				if (res != _ov.end() && !(_ov.get_key(*res) > key))
					Assert::Fail();
			}
		}
		TEST_METHOD(equal_bound) {
			for (size_t i = 0; i < _keys.size(); ++i) {
				auto& key = _keys[i];
				auto eb = _ov.equal_range(key);

				for (auto it = eb.first; it != eb.second; ++it) {
					if (it != _ov.end() && !(_ov.get_key(*it) == key))
						Assert::Fail();
				}
			}
		}

	};

}
