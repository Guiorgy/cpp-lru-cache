/*
 * File:   lrucache.hpp
 * Authors: Alexander Ponomarev, Guiorgy
 *
 * Created on June 20, 2013, 5:09 PM
 * Updated on October 23, 2024
 */

#pragma once

#include <unordered_map>
#include <optional>
#include <list>

template<typename key_t, typename value_t, const size_t max_size>
class lru_cache {
public:
	typedef typename std::pair<key_t, value_t> key_value_pair_t;
	typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

private:
	std::list<key_value_pair_t> _cache_items_list;
	std::unordered_map<key_t, list_iterator_t> _cache_items_map;

	void put(const key_t& key) {
		remove(key);
		_cache_items_map[key] = _cache_items_list.begin();

		if (_cache_items_map.size() > max_size) {
			auto last = _cache_items_list.end();
			--last;
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
			return {};
		} else {
			_cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
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

	size_t size() const noexcept {
		return _cache_items_map.size();
	}

	void clear() noexcept {
		_cache_items_map.clear();
		_cache_items_list.clear();
	}
};
