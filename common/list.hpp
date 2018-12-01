//
//  list.hpp
//  index
//
//  Created by 猪摇摇 on 2018/11/12.
//  Copyright © 2018 bfzq. All rights reserved.
//

#ifndef list_hpp
#define list_hpp

#include <stdio.h>

#include <functional>




namespace bfzq {
	template <typename V>
	struct Node {
		V v ;
		Node<V>* per ;
		Node<V>* next ;
		Node(V v) : v(v),per(nullptr),next(nullptr) {}
	};
	
	
	template <typename T>
	class List {
	public:
		List() = default;
		List(const List&) ;
		~List() ;
	public:
		void Insert(T) ;
		bool Delete(T&) ;
		bool Delete(unsigned int index) ;
		void foreach(std::function<void(T)>) const;
		unsigned int size() ;
		void clean() ;
		
	private:
		Node<T>* _head = nullptr;
		Node<T>* _tail = nullptr;
		unsigned int _length = 0;
		
	} ;
	
	template <typename T>
	List<T>::List(const List& list) {
		list.foreach([this](T t) {
			Insert(t) ;
		}) ;
	}
	
	template <typename T>
	List<T>::~List() {
		clean() ;
	}
	
	template <typename T>
	void List<T>::Insert(T t) {
		Node<T>* node = new Node<T>(t) ;
		if (!_head) {
			_head = node ;
			_tail = _head ;
		} else {
			node->per = _tail ;
			_tail->next = node ;
			_tail = node ;
		}
		_length++ ;
		node = nullptr ;
	}
	
	template <typename T>
	bool List<T>::Delete(T& t) {
		unsigned int idx = 0 ;
		foreach([&idx,&t](T node) {
			if (node == *t) {
				return ;
			}
			idx++ ;
		}) ;
		return Delete(idx) ;
	}
	
	template <typename T>
	bool List<T>::Delete(unsigned int index) {
		if (index > _length) return false ;
		Node<T>* finger = _head ;
		for (unsigned int i = 0; i < _length && finger->next != nullptr; i++, finger = finger->next) {
			if (i == index) {
				if (finger == _head) {
					if (_tail == _head) {
						_tail = _head->next ;
					}
					_head = _head->next ;
					_head->per = nullptr ;
					delete finger ;
				}else {
					finger->per->next = finger->next ;
					finger->next->per = finger->per ;
					delete finger ;
				}
				finger = nullptr ;
				_length-- ;
				return true ;
			}
		}
		finger = nullptr ;
		return false ;
	}
	
	template <typename T>
	void List<T>::foreach(std::function<void(T)> func) const{
		Node<T>* node = _head ;
		if (!node) return ;
		do {
			func(node->v) ;
		} while (nullptr != (node = node->next));
	}
	
	
	template <typename T>
	unsigned int List<T>::size() {
		return _length ;
	}
	
	template <typename T>
	void List<T>::clean() {
		while (_tail) {
			if (_tail->per != nullptr) {
				_tail = _tail->per ;
				delete _tail->next ;
				_tail->next = nullptr ;
			} else {
				delete _tail ;
				_tail = nullptr ;
			}
		}
	}
}

#endif /* list_hpp */
