/*
 * File: lrucache.hpp
 * Authors: Alexander Ponomarev, Guiorgy
 *
 * Created on June 20, 2013, 5:09 PM
 * Updated on October 23, 2024
 */

#pragma once

#include <unordered_map>
#include <functional>
#include <optional>
#include <utility>
#include <tuple>
#include <list>

#if __cplusplus < 202002L	// C++20
	#ifdef nodiscard
		#define GUIORGY_nodiscard_BEFORE
		#undef nodiscard
	#endif
	#define nodiscard(explanation) nodiscard
#endif

template<typename key_t, typename value_t, const std::size_t max_size>
class lru_cache final {
	static_assert(max_size > 0);

	typedef typename std::pair<key_t, value_t> key_value_pair_t;
	typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;
	typedef typename std::list<key_value_pair_t>::reverse_iterator list_reverse_iterator_t;
	typedef typename std::unordered_map<key_t, list_iterator_t>::iterator map_iterator_t;

public:
	typedef typename std::list<key_value_pair_t>::const_iterator const_iterator;
	typedef typename std::list<key_value_pair_t>::const_reverse_iterator const_reverse_iterator;

private:
	std::list<key_value_pair_t> _cache_items_list;
	std::unordered_map<key_t, list_iterator_t> _cache_items_map;

public:
	lru_cache() = default;
	~lru_cache() = default;
	lru_cache(const lru_cache& other) {
		_cache_items_list = other._cache_items_list;

		_cache_items_map.reserve(_cache_items_list.size());
		for (list_iterator_t it = _cache_items_list.begin(); it != _cache_items_list.end(); ++it) {
			_cache_items_map.emplace_hint(_cache_items_map.end(), it->first, it);
		}
	}
	lru_cache(lru_cache&&) = default;
	lru_cache& operator=(lru_cache const& other) {
		_cache_items_list = other._cache_items_list;

		_cache_items_map.reserve(_cache_items_list.size());
		for (list_iterator_t it = _cache_items_list.begin(); it != _cache_items_list.end(); ++it) {
			_cache_items_map.emplace_hint(_cache_items_map.end(), it->first, it);
		}

		return *this;
	}
	lru_cache& operator=(lru_cache &&) = default;

private:
	void put(const key_t& key) {
		map_iterator_t it = _cache_items_map.find(key);

		if (it != _cache_items_map.end()) {
			_cache_items_list.erase(it->second);
			it->second = _cache_items_list.begin();
		} else {
			_cache_items_map.emplace_hint(it, key, _cache_items_list.begin());
		}

		if (_cache_items_map.size() > max_size) {
			list_reverse_iterator_t last = _cache_items_list.rbegin();
			_cache_items_map.erase(last->first);
			_cache_items_list.pop_back();
		}
	}

public:
	void put(const key_t& key, const value_t& value) {
		_cache_items_list.emplace_front(key, value);
		put(key);
	}

	void put(const key_t& key, value_t&& value) {
		_cache_items_list.emplace_front(key, std::move(value));
		put(key);
	}

	template<typename... ValueArgs>
	const value_t& emplace(const key_t& key, ValueArgs&&... value_args) {
		const value_t& value = _cache_items_list.emplace_front(
			std::piecewise_construct,
			std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<ValueArgs>(value_args)...)
		).second;
		put(key);
		return value;
	}

	[[nodiscard]] const std::optional<value_t> get(const key_t& key) {
		map_iterator_t it = _cache_items_map.find(key);

		if (it == _cache_items_map.end()) {
			return std::nullopt;
		} else {
			if (it->second != _cache_items_list.begin()) {
				_cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
			}
			return it->second->second;
		}
	}

	[[nodiscard]] const std::optional<std::reference_wrapper<const value_t>> get_ref(const key_t& key) {
		map_iterator_t it = _cache_items_map.find(key);

		if (it == _cache_items_map.end()) {
			return std::nullopt;
		} else {
			if (it->second != _cache_items_list.begin()) {
				_cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
			}
			return std::make_optional(std::cref(it->second->second));
		}
	}

	[[nodiscard]] bool try_get(const key_t& key, value_t& value_out) {
		map_iterator_t it = _cache_items_map.find(key);

		if (it == _cache_items_map.end()) {
			return false;
		} else {
			if (it->second != _cache_items_list.begin()) {
				_cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
			}
			value_out = it->second->second;
			return true;
		}
	}

	[[nodiscard]] bool try_get_ref(const key_t& key, const value_t*& value_out) {
		map_iterator_t it = _cache_items_map.find(key);

		if (it == _cache_items_map.end()) {
			value_out = nullptr;
			return false;
		} else {
			if (it->second != _cache_items_list.begin()) {
				_cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
			}
			value_out = &(it->second->second);
			return true;
		}
	}

	void remove(const key_t& key) {
		map_iterator_t it = _cache_items_map.find(key);

		if (it != _cache_items_map.end()) {
			_cache_items_list.erase(it->second);
			_cache_items_map.erase(it);
		}
	}

	[[nodiscard]] bool exists(const key_t& key) const {
		return _cache_items_map.find(key) != _cache_items_map.end();
	}

	[[nodiscard]] std::size_t size() const noexcept {
		return _cache_items_map.size();
	}

	void clear() noexcept {
		_cache_items_map.clear();
		_cache_items_list.clear();
	}

	[[nodiscard]] const_iterator cbegin() const noexcept {
		return _cache_items_list.cbegin();
	}

	[[nodiscard]] const_iterator begin() const noexcept {
		return cbegin();
	}

	[[nodiscard]] const_iterator cend() const noexcept {
		return _cache_items_list.cend();
	}

	[[nodiscard]] const_iterator end() const noexcept {
		return cend();
	}

	[[nodiscard]] const_reverse_iterator crbegin() const noexcept {
		return _cache_items_list.crbegin();
	}

	[[nodiscard]] const_iterator rbegin() const noexcept {
		return crbegin();
	}

	[[nodiscard]] const_reverse_iterator crend() const noexcept {
		return _cache_items_list.crend();
	}

	[[nodiscard]] const_iterator rend() const noexcept {
		return crend();
	}
};

// Restore nodiscard if it was already defined.
#ifdef GUIORGY_nodiscard_BEFORE
	#undef nodiscard
	#define nodiscard GUIORGY_nodiscard_BEFORE
	#undef GUIORGY_nodiscard_BEFORE
#endif
