#ifndef COMPONENT_STORAGE_H
#define COMPONENT_STORAGE_H

#include <string>

namespace NovaEngine {
	class ComponentStorage {
	private:

	public:

		template<typename A>
		inline A create() {
			return new A();
		}

		inline void get() {
			
		}
	};
}

#endif // !COMPONENT_STORAGE_H