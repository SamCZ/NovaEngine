#ifndef POST_PROCESS_H
#define POST_PROCESS_H

namespace NovaEngine {
	class PostProcess {
	private:

	public:
		PostProcess();

		void init();
		void render();

		bool onGameObjectRequested(GameObject* obj);

	};
}

#endif // !POST_PROCESS_H