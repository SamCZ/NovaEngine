#ifndef ID_PROVIDER_H
#define ID_PROVIDER_H

namespace NovaEngine {
	class IdProvider {
	private:
		int _nextId;
	public:
		inline IdProvider() {
			_nextId = 0;
		}

		inline int getId() {
			return _nextId++;
		}

		inline void clear() {
			_nextId = 0;
		}
	};
}

#endif // !ID_PROVIDER_H