#pragma once
#pragma GCC optimize("Ofast")
#include <algorithm>
#include <cmath>
#include <iostream>
#include <initializer_list>
#include <list>
#include <vector>
using namespace std;

// Hash-table using open adressing + Robin Hood optimization
template<class KeyType, class ValueType, class Hash = hash<KeyType>> class HashMap {
public:

	HashMap(Hash hasher = Hash()) : hasher_(hasher) {
		Init(min_cap_);
	}

	template<class InputIt>
	HashMap(InputIt begin, InputIt end, Hash hasher = Hash()) : hasher_(hasher) {
		Init(min_cap_);
		for (InputIt it = begin; it != end; it++) {
			insert(*it);
		}
	}

	HashMap(initializer_list<pair<KeyType, ValueType>> il, Hash hasher = Hash()) : hasher_(hasher) {
		Init(min_cap_);
		for (auto it : il) {
			insert(it);
		}
	}

	HashMap& operator=(const HashMap& other) {
		bucket_ = other.bucket_;
		used_ = other.used_;
		end_elem_ = other.end_elem_;
		size_ = other.size_;
		capacity_ = other.capacity_;
		hasher_ = other.hasher_;
		return *this;
	}

	HashMap(const HashMap& other) {
		*this = other;
	}

	size_t size() const {
		return size_;
	}

	bool empty() const {
		return size_ == 0;
	}

	Hash hash_function() const {
		return hasher_;
	}

	void insert(pair<KeyType, ValueType> elem) {
		short int cur_dist = 0;
		size_t pos = hasher_(elem.first) % capacity_;
		pair<KeyType, ValueType> x = elem;

		if (find(elem.first) != end()) {
			return;
			//already exists
		}

		while (used_[pos]) {
			if (cur_dist > dist_[pos]) {
				swap(cur_dist, dist_[pos]);
				swap(x, bucket_[pos]);
			}
			cur_dist++;
			pos = MoveHash(pos);
		}

		bucket_[pos] = x;
		used_[pos] = true;
		size_++;
		if (size_ >= capacity_ * r_alpha_) {
			SetCapacity(capacity_ * 2);
		}
	}

	void erase(KeyType key) {
		size_t pos = hasher_(key) % capacity_;
		bool find = false;
		size_t prev_empty = 0;

		while (used_[pos]) {
			if (bucket_[pos].first == key) {
				// erase element
				find = true;
				size_--;
				used_[pos] = false;
				prev_empty = pos;
			}
			else if (find) {
				// moving other elements to new empty space
				size_t h = hasher_(bucket_[pos].first) % capacity_;
				if (h != pos) {
					bool f = false;
					size_t pos1 = h;
					while (pos1 != pos) {
						if (pos1 == prev_empty) {
							f = true;
							break;
						}
						pos1 = MoveHash(pos1);
					}
					if (f) {
						swap(bucket_[pos], bucket_[prev_empty]);
						swap(used_[pos], used_[prev_empty]);
						prev_empty = pos;
					}
				}
			}
			pos = MoveHash(pos);
		}

		if (size_ <= capacity_ * l_alpha_) {
			SetCapacity(max(min_cap_, capacity_ / 2));
		}
	}

	class iterator {
	public:
		iterator() {
			pos = 0;
		}
		iterator(size_t x, const HashMap* hashmap) {
			pos = x;
			hm = hashmap;
		}

		pair<const KeyType, ValueType>& operator*() {
			return (pair<const KeyType, ValueType>&)(pos < hm->capacity_ ? hm->bucket_[pos] : hm->end_elem_);
		}
		pair<const KeyType, ValueType>* operator->() {
			if (pos < hm->capacity_) {
				return (pair<const KeyType, ValueType>*)(&(hm->bucket_[pos]));
			}
			else {
				return (pair<const KeyType, ValueType>*)(&(hm->end_elem_));
			}
		}

		iterator& operator++() {
			for (int i = 0; i < 1 || (pos < hm->capacity_ && !hm->used_[pos]); i++) {
				pos++;
			}
			return *this;
		}
		iterator operator++(int) {
			iterator it = *this;
			++(*this);
			return it;
		}

		bool operator==(iterator other) const {
			return pos == other.pos;
		}
		bool operator!=(iterator other) const {
			return !(*this == other);
		}

	private:
		size_t pos;
		const HashMap* hm;

	};

	class const_iterator {
	public:

		const_iterator() {
			this->it = iterator();
		}
		const_iterator(size_t x, const HashMap* hashmap) {
			it = iterator(x, hashmap);
		}
		const_iterator(iterator it) {
			this->it = it;
		}

		const pair<const KeyType, ValueType>& operator*() {
			return *it;
		}
		const pair<const KeyType, ValueType>* operator->() {
			return it.operator->();
		}

		const_iterator& operator++() {
			it++;
			return *this;
		}
		const_iterator operator++(int) {
			const_iterator cit = *this;
			++(*this);
			return cit;
		}

		bool operator==(const const_iterator other) const {
			return it == other.it;
		}
		bool operator!=(const const_iterator other) const {
			return it != other.it;
		}
	private:
		iterator it;
	};

	iterator begin() {
		if (size_ == 0) {
			return end();
		}
		size_t pos = 0;
		while (!used_[pos]) {
			pos = (pos + 1) % capacity_;
		}
		return iterator(pos, this);
	}
	const_iterator begin() const {
		if (size_ == 0) {
			return end();
		}
		size_t pos = 0;
		while (!used_[pos]) {
			pos = (pos + 1) % capacity_;
		}
		return const_iterator(pos, this);
	}

	iterator end() {
		return iterator(capacity_, this);
	}
	const_iterator end() const {
		return const_iterator(capacity_, this);
	}

	iterator find(KeyType key) {
		size_t pos = hasher_(key) % capacity_;
		while (used_[pos] && !(bucket_[pos].first == key)) {
			pos = MoveHash(pos);
		}
		if (used_[pos]) {
			return iterator(pos, this);
		}
		else {
			return end();
		}
	}
	const_iterator find(KeyType key) const {
		size_t pos = hasher_(key) % capacity_;
		while (used_[pos] && !(bucket_[pos].first == key)) {
			pos = MoveHash(pos);
		}
		if (used_[pos]) {
			iterator bruh(pos, this);
			return const_iterator(bruh);
		}
		else {
			return end();
		}
	}

	ValueType& operator[](KeyType key) {
		ValueType vt = ValueType(); // default value
		insert({ key, vt });
		iterator it = find(key);
		return (*it).second;
	}
	const ValueType& at(KeyType key) const {
		const_iterator it = find(key);
		if (it != end()) {
			return it->second;
		}
		else {
			throw out_of_range("");
		}
	}

	void clear() {
		for (size_t i = 0; i < capacity_; i++) {
			used_[i] = 0;
		}
		size_ = 0;
		capacity_ = min_cap_;
		bucket_.resize(capacity_);
		used_.resize(capacity_);
		dist_.resize(capacity_);
	}

private:

	size_t MoveHash(size_t pos) const {
		return (pos + 1) % capacity_;
	}

	void Init(size_t cap) {
		size_ = 0;
		capacity_ = cap;
		bucket_.resize(capacity_);
		used_.resize(capacity_);
		dist_.resize(capacity_);
	}

	void SetCapacity(size_t new_cap) { // resize bucket and recount all keys
		vector <pair<KeyType, ValueType>> elem_list;
		elem_list.reserve(size_);
		for (size_t i = 0; i < capacity_; i++) {
			if (used_[i]) {
				elem_list.push_back(bucket_[i]);
				used_[i] = false;
			}
		}
		bucket_.resize(new_cap);
		used_.resize(new_cap);
		dist_.resize(new_cap);
		capacity_ = new_cap;
		size_ = 0;
		for (auto& i : elem_list) {
			insert(i);
		}
	}

	vector <pair<KeyType, ValueType>> bucket_; // elements
	vector <bool> used_; // for deleted/empty elements
	vector <short int> dist_; // for Robin Hood hashing
	pair <KeyType, ValueType> end_elem_ = pair<KeyType, ValueType>(); // for end() iterator
	size_t size_ = 0;
	size_t capacity_ = 8;
	Hash hasher_;

	const size_t min_cap_ = 8;
	const double r_alpha_ = 0.7;
	const double l_alpha_ = 0.2;
};
