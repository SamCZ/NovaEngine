#ifndef SCROLL_PANE_RENDERER_H
#define SCROLL_PANE_RENDERER_H


#include "Engine/UI/TypeRenderer.h"

#include "Engine/Input/InputManager.h"
#include "Engine/Utils/Files.h"
#include "Engine/UI/Types/ScrollPane.h"

namespace NovaEngine {
	class ScrollPaneRendeer : public TypeRenderer {
	private:
		ScrollPane* _pane;
		int _scrollContainerSize;
		int _scrollBarSize;
		int _scrollContainerSpace;



	public:
		inline ScrollPaneRendeer(ScrollPane* pane) {
			_pane = pane;

			_scrollContainerSize = 14;
			_scrollBarSize = 7;

			_scrollContainerSpace = _scrollContainerSize / 2 - _scrollBarSize / 2;
		}

		inline void render(UIRenderer* renderer, UINode* node) override {
			if (_pane->haveVertical()) {
				int scrollCVX = _pane->getWorldX() + _pane->getWidth() - _scrollContainerSize;
				int scrollCVY = _pane->getWorldY();
				renderer->drawRect(scrollCVX, scrollCVY, _scrollContainerSize, _pane->getHeight(), makeHEX("EAEAEA"));
				renderer->drawRect(scrollCVX, scrollCVY, 1, _pane->getHeight(), makeHEX("999999"));
				renderer->drawRect(scrollCVX + _scrollContainerSpace, scrollCVY + _scrollContainerSpace, _scrollBarSize, 100, makeHEX("A1A1A1"), makeBorder(0.5f, makeRGB(20, 20, 20), makeRadius(2.5f)));
			}

			if (_pane->haveHorizontal()) {
				int scrollCHX = _pane->getWorldX();
				int scrollCHY = _pane->getWorldY() + _pane->getHeight() - _scrollContainerSize;
				renderer->drawRect(scrollCHX, scrollCHY, _pane->getWidth(), _scrollContainerSize, makeHEX("EAEAEA"));
				if (!_pane->haveVertical()) {
					renderer->drawRect(scrollCHX, scrollCHY, _pane->getWidth(), 1, makeHEX("999999"));
				} else {
					renderer->drawRect(scrollCHX, scrollCHY, _pane->getWidth() - _scrollContainerSize + 1, 1, makeHEX("999999"));
				}
				renderer->drawRect(scrollCHX + _scrollContainerSpace, scrollCHY + _scrollContainerSpace, 100, _scrollBarSize, makeHEX("A1A1A1"), makeBorder(0.5f, makeRGB(20, 20, 20), makeRadius(2.5f)));
			}
		}
	};
}

#endif // !SCROLL_PANE_RENDERER_H