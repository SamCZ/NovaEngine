#ifndef SCROLL_PANE_H
#define SCROLL_PANE_H

#include "Engine/UI/UINode.h"

namespace NovaEngine {
	class ScrollPaneRendeer;

	class ScrollPane : public UINode {
	private:
		ScrollPaneRendeer* _renderer;
		UINode* _contentPane;
		bool _vertical;
		bool _horizontal;
	public:
		ScrollPane(UINode* parent, bool vertical, bool horizontal);

		int getWidth();
		int getHeight();

		void addChild(UINode* node);
		void removeChild(UINode* node);

		bool haveVertical() const;
		bool haveHorizontal() const;
	};
}

#endif // !SCROLL_PANE_H