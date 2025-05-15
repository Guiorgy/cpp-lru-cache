/*
 * File: lrucache.hpp
 * Authors: Alexander Ponomarev, Guiorgy
 *
 * Created on June 20, 2013, 5:09 PM
 * Updated on October 23, 2024
 */

#pragma once

#include <unordered_map>
#include <optional>
#include <list>

template<typename key_t, typename value_t, const std::size_t max_size>
class lru_cache final {
	static_assert(max_size > 0);

	typedef typename std::pair<key_t, value_t> key_value_pair_t;
	typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

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
		for (auto it = _cache_items_list.begin(); it != _cache_items_list.end(); ++it) {
			_cache_items_map[it->first] = it;
		}
	}
	lru_cache(lru_cache&&) = default;
	lru_cache& operator=(lru_cache const& other) {
		_cache_items_list = other._cache_items_list;

		_cache_items_map.reserve(_cache_items_list.size());
		for (auto it = _cache_items_list.begin(); it != _cache_items_list.end(); ++it) {
			_cache_items_map[it->first] = it;
		}

		return *this;
	}
	lru_cache& operator=(lru_cache &&) = default;

private:
	void put(const key_t& key) {
		auto it = _cache_items_map.find(key);
		if (it != _cache_items_map.end()) {
			_cache_items_list.erase(it->second);
			it->second = _cache_items_list.begin();
		} else {
			_cache_items_map[key] = _cache_items_list.begin();
		}

		if (_cache_items_map.size() > max_size) {
			auto last = _cache_items_list.rbegin();
			_cache_items_map.erase(last->first);
			_cache_items_list.pop_back();
		}
	}

public:
	void put(const key_t& key, const value_t& value) {
		_cache_items_list.push_front(key_value_pair_t(key, value));
		put(key);
	}

	void put(const key_t& key, value_t&& value) {
		_cache_items_list.emplace_front(key, std::move(value));
		put(key);
	}

	const std::optional<value_t> get(const key_t& key) {
		auto it = _cache_items_map.find(key);

		if (it == _cache_items_map.end()) {
			return std::nullopt;
		} else {
			if (it->second != _cache_items_list.begin()) {
				_cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
			}
			return it->second->second;
		}
	}

	void remove(const key_t& key) {
		auto it = _cache_items_map.find(key);
		if (it != _cache_items_map.end()) {
			_cache_items_list.erase(it->second);
			_cache_items_map.erase(it);
		}
	}

	bool exists(const key_t& key) const {
		return _cache_items_map.find(key) != _cache_items_map.end();
	}

	std::size_t size() const noexcept {
		return _cache_items_map.size();
	}

	void clear() noexcept {
		_cache_items_map.clear();
		_cache_items_list.clear();
	}

	const_iterator cbegin() const noexcept {
		return _cache_items_list.cbegin();
	}

	const_iterator begin() const noexcept {
		return cbegin();
	}

	const_iterator cend() const noexcept {
		return _cache_items_list.cend();
	}

	const_iterator end() const noexcept {
		return cend();
	}

	const_reverse_iterator crbegin() const noexcept {
		return _cache_items_list.crbegin();
	}

	const_iterator rbegin() const noexcept {
		return crbegin();
	}

	const_reverse_iterator crend() const noexcept {
		return _cache_items_list.crend();
	}

	const_iterator rend() const noexcept {
		return crend();
	}
};
