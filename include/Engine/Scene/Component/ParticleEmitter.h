#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include "Engine/Scene/Component/Component.h"

namespace NovaEngine {
	class ParticleEmitter : public Component {
	private:
		TVar<float> dataFloat;
		UVar button;

	public:
		inline ~ParticleEmitter() {

		}

		inline ParticleEmitter() : Component("Particle Emitter"), dataFloat("Data float", 1.0f), button("Test") {
			addVar(dataFloat);
			addVar(button);
		}

		inline void init() override {

		}

		inline void update() override {

		}
	};
}

#endif // !PARTICLE_EMITTER_H