#ifndef LIST_H
#define LIST_H

#include <vector>
#include <iostream>

namespace NovaEngine {
	template<typename T>
	class List {
	public:
		List() : _storage() {}
		List(int size) : _storage(size) {}

		inline void add(T obj) {
			_storage.push_back(obj);
		}

		inline void remove(T obj) {
			_storage.erase(std::find(_storage.begin(), _storage.end(), obj));
		}

		inline int size() {
			return _storage.size();
		}

		inline T get(int index) {
			return _storage[index];
		}

		inline void clear() {
			_storage.clear();
		}
	private:
		std::vector<T> _storage;
	};
}

#endif // !LIST_H