#ifndef RENDER_STATE_H
#define RENDER_STATE_H

namespace NovaEngine {
	enum class BlendMode {
		Off,
		Additive,
		PremultAlpha,
		AlphaAdditive,
		Color,
		Alpha,
		Screen
	};

	enum class FaceCull {
		Off,
		Front,
		Back,
		FrontAndBack
	};

	struct RenderState {
		BlendMode blendMode = BlendMode::Off;
		FaceCull faceCull = FaceCull::Back;
		bool wireframe = false;
		bool depthTest = true;
		float lineWidth = 1.0f;
	};

	extern RenderState DefaultRenderState;
}

#endif // !RENDER_STATE_H