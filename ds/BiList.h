#pragma once

#include <string>

// T should be a class with
// T *prev, *next;
// void reset();

template <typename T>
class BiList {
private:
	T* list;
	T* tail;
	size_t sz;
public:
	BiList() {
		list = nullptr;
		tail = nullptr;
		sz = 0;
	}
	T* head() {
		return list;
	}
	size_t size() {
		return sz;
	}
	void push_back(T* node) {
		if (list == nullptr) {
			list = node;
			tail = node;
			node->prev = nullptr;
			node->next = nullptr;
		}
		else {
			tail->next = node;
			node->prev = tail;
			node->next = nullptr;
			tail = node;
		}
		sz++;
	}
	void erase(T* node, std::string type) {
		assert(node != nullptr && "BiList::erase: pointer is nullptr");
		if (node == list) {
			list = list->next;
			if (list != nullptr) {
				list->prev = nullptr;
			}
		}
		else if (node == tail){
			tail = tail->prev;
			if (tail != nullptr) {
				tail->next = nullptr;
			}
		}
		else {
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}
		sz--;
		if (type == "delete") {
			node->reset();
			delete node;
		}
	}
	void delete_all() {
		T* next_ptr;
		for (T* ptr = list; ptr != nullptr; ptr = next_ptr) {
			next_ptr = ptr->next;
			ptr->reset();
			delete ptr;
		}
		list = nullptr;
		tail = nullptr;
		sz = 0;
	}
};