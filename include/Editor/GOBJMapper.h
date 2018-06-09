#ifndef GOBJ_MAPPER_H
#define GOBJ_MAPPER_H

#include <string>
#include <map>

namespace NovaEngine {
	class GOBJMapper {
	public:
		std::map<int, std::string> MaterialMap;
	};
}

#endif // !GOBJ_MAPPER_H