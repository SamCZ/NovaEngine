#ifndef SCRIPTING_H
#define SCRIPTING_H

#include "Engine/Scene/Spatial.h"

namespace NovaEngine {
	class Scripting {
	private:
		virtual void initSystem() = 0;
	public:
		inline ~Scripting() { }

		virtual void init(Spatial* scene) = 0;
		virtual void update(Spatial* scene) = 0;
	};
}

#endif // !SCRIPTING_H