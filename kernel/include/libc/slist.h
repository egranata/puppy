/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBC_SLIST
#define LIBC_SLIST

#include <kernel/sys/stdint.h>
#include <kernel/libc/function.h>

template<typename T>
class slist {
private:
	struct node {
		T item;
		node* next;
	};
	size_t size;
	node *head;
	node *tail;

	void clone(const slist<T>& other) {
		auto ob = other.begin();
		auto oe = other.end();
		while (ob != oe) {
			add(*ob);
			++ob;
		}
	}
public:
	slist() : size(0), head(nullptr), tail(nullptr) {}

	explicit slist(const slist<T>& other) : size(0), head(nullptr), tail(nullptr) {
		clone(other);
	}

	slist<T>& operator=(const slist<T>& other) {
		clear();
		clone(other);
		return *this;
	}

    bool empty() const { return size == 0; }

	size_t count() const { return size; }

	void add_head(T item) {
		++size;
		if (head == nullptr) {
			head = new node{item,nullptr}; 
			tail = head;			
		} else {
			auto n = new node{item, head};
			head = n;
		}
	}

	void add(T item) {
		++size;
		if (head == nullptr) {
			head = new node{item,nullptr}; 
			tail = head;
		} else {
			tail->next = new node{item, nullptr};
			tail = tail->next;
		}
	}
	
	class iterator {
	public:
		T& operator*() { return current->item; }
		T& operator->() { return current->item; }
		iterator& operator++() {
			current = current->next;
			return *this;
		}
		bool operator==(const iterator& i) {
			return current == i.current;
		}
		bool operator!=(const iterator& i) {
			return current != i.current;
		}
	private:
		iterator(node *n) : current(n) {}
		node* current;
		
		friend class slist;
	};

	class const_iterator {
	public:
		const T& operator*() { return current->item; }
		T& operator->() { return current->item; }
		const_iterator& operator++() {
			current = current->next;
			return *this;
		}
		bool operator==(const const_iterator& i) {
			return current == i.current;
		}
		bool operator!=(const const_iterator& i) {
			return current != i.current;
		}
	private:
		const_iterator(node *n) : current(n) {}
		node* current;
		
		friend class slist;
	};

	iterator begin() { return iterator(head); }
	iterator end() { return empty() ? begin() : iterator(tail->next); }
	
	const_iterator begin() const { return const_iterator(head); }
	const_iterator end() const { return empty() ? begin() : const_iterator(tail->next); }

	void clear() {
		size = 0;
		while (head != nullptr) {
			auto m = head->next;
			delete head;
			head = m;
		}
		head = tail = nullptr;
	}

	void insert(iterator i, T item) {
		if (i.current) {
			++size;
			auto n = i.current;
			auto newnode = new node{n->item, n->next};
			n->item = item;
			n->next = newnode;
			if (n == tail) tail = newnode;
		} else {
			add(item);
		}
	}

	void remove(const iterator& i) {
		if (i.current) {
			if (i.current == head) {
				head = head->next;
				--size;
			} else {
				node *n = head;
				for (; n->next != i.current; n = n->next);
				if (i.current == tail) {
					n->next = nullptr;
					tail = n;
				} else {
					n->next = n->next->next;
				}
				--size;
				delete i.current;
			}
		}
	}
	
	bool remove(const T& item) {
		auto b = begin(), e = end();
		for(; b != e; ++b) {
			if (*b == item) { remove(b); return true; }
		}
		return false;
	}

	void removeAll(const T& item) {
		while(remove(item));
	}

	T top() {
		return head->item;
	}
	
	T back() {
		return tail->item;
	}

	T pop() {
		--size;
		auto n = head;
		head = head->next;
        auto t = n->item;
		delete n;
        return t;
	}

	void foreach(function<bool(T&)> f) {
		auto b = begin(), e = end();
		while(b != e) {
			if (false == f(*b)) return;
			++b;
		}
	}

	void foreach(function<bool(const T&)> f) const {
		auto b = begin(), e = end();
		while(b != e) {
			if (false == f(*b)) return;
			++b;
		}
	}
};

#endif
