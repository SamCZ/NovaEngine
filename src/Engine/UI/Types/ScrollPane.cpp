#include "Engine/UI/Types/ScrollPane.h"
#include "Engine/UI/Renderers/ScrollPaneRenderer.h"

namespace NovaEngine {
	ScrollPane::ScrollPane(UINode* parent, bool vertical, bool horizontal) : UINode() {
		_vertical = vertical;
		_horizontal = horizontal;
		parent->addChild(this);

		_contentPane = new UINode();
		_contentPane->setParent(this);
		_childs.push_back(_contentPane);

		//_contentPane->setClip(20, getHeight());

		_renderer = new ScrollPaneRendeer(this);
		addRenderer(_renderer, 0);
	}

	int ScrollPane::getWidth() {
		return getParent()->getWidth();
	}

	int ScrollPane::getHeight() {
		return getParent()->getHeight();
	}

	void ScrollPane::addChild(UINode* node) {
		_contentPane->addChild(node);
	}

	void ScrollPane::removeChild(UINode* node) {
		_contentPane->removeChild(node);
	}

	bool ScrollPane::haveVertical() const {
		return _vertical;
	}

	bool ScrollPane::haveHorizontal() const {
		return _horizontal;
	}

}