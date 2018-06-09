#include "Engine/UI/UINode.h"
#include "Engine/UI/TypeRenderer.h"
#include "Engine/Utils/Utils.h"

namespace NovaEngine {

	UINode::UINode() : _parent(nullptr), _cursor(CursorType::Arrow) { }

	UINode::~UINode() { }

	void UINode::setX(int x) {
		_x = x;
	}

	void UINode::setY(int y) {
		_y = y;
	}

	int UINode::getX() const {
		return _x;
	}

	int UINode::getY() const {
		return _y;
	}

	int UINode::getWorldX() {
		if (_parent != nullptr) {
			return _parent->getWorldX() + _x;
		}
		return _x;
	}

	int UINode::getWorldY() {
		if (_parent != nullptr) {
			return _parent->getWorldY() + _y;
		}
		return _y;
	}

	void UINode::setWidth(int width) {
		_width = width;
	}

	void UINode::setHeight(int height) {
		_height = height;
	}

	int UINode::getWidth() const {
		return _width;
	}

	int UINode::getHeight() const {
		return _height;
	}

	void UINode::setSize(int width, int height) {
		_width = width;
		_height = height;
	}

	void UINode::setClip(bool flag) {
		_clipEnabled = flag;
	}

	void UINode::setClip(int width, int height) {
		ClipRect c;
		c.width = width;
		c.height = height;
		_clipRect = c;
		_clipEnabled = true;
	}

	bool UINode::isClipEnabled() {
		return _clipEnabled;
	}

	ClipRect& UINode::getClip() {
		return _clipRect;
	}

	bool UINode::isZero() const {
		return getWidth() == 0 && getHeight() == 0;
	}

	void UINode::setFocused(bool focused) {
		_focused = focused;
	}

	void UINode::setClicked(bool clicked) {
		_clicked = clicked;
	}

	void UINode::setVisible(bool visible) {
		_visible = visible;
	}

	void UINode::setEnabled(bool enabled) {
		_enabled = enabled;
	}

	bool UINode::isFocused() {
		return _focused && isVisible() && isEnabled();
	}

	bool UINode::isClicked() {
		return _clicked && isVisible() && isEnabled();
	}

	bool UINode::isVisible() {
		if (_parent != nullptr) {
			return _visible && _parent->isVisible();
		}
		return _visible;
	}

	bool UINode::isEnabled() {
		return _enabled;
	}

	void UINode::setParent(UINode* parent) {
		_parent = parent;
	}

	UINode* UINode::getParent() {
		return _parent;
	}

	void UINode::setId(std::string id) {
		_id = id;
	}

	std::string UINode::getId() {
		return _id;
	}

	void UINode::addChild(UINode* node) {
		node->setParent(this);
		_childs.push_back(node);
	}

	void UINode::removeChild(UINode* node) {
		_childs.erase(std::find(_childs.begin(), _childs.end(), node));
	}

	int UINode::getChildCount() {
		return _childs.size();
	}

	std::vector<UINode*>& UINode::getChilds() {
		return _childs;
	}

	void UINode::setCursor(const CursorType& cursor) {
		_cursor = cursor;
	}

	CursorType UINode::getCursor() {
		return _cursor;
	}

	void UINode::addRenderer(TypeRenderer* renderer, int layer) {
		renderer->R_INDEX = layer;
		_renderers.push_back(renderer);
		if (_renderers.size() > 1) {
			mergeSort<TypeRenderer*>(_renderers, [](TypeRenderer* left, TypeRenderer* right) {
				return left->R_INDEX < right->R_INDEX ? -1 : 1;
			});
		}
	}

	TypeRenderer* UINode::getRendrer(int index) {
		return _renderers[index];
	}

	int UINode::getRenderersCount() const {
		return _renderers.size();
	}

	std::vector<TypeRenderer*>& UINode::getRenderers() {
		return _renderers;
	}

	void UINode::onMouseMove(int x, int y, int dx, int dy) {
		/*if (_onFocusListener != NULL) {
			_onFocusListener(isFocused());
		}*/
		if (!isZero() && UINode::intersects(x, y, this)) {
			setFocused(true);
			return;
		}
		setFocused(false);
	}

	void UINode::onMouseEvent(ButtonEvent e) {
		if (e.state && isFocused() && isEnabled() && isVisible()) {
			setClicked(true);
			if (_onClickListener != NULL) {
				_onClickListener();
			}
		} else {
			setClicked(false);
		}

	}

	void UINode::onKeyEvent(KeyEvent e) {

	}

	void UINode::onMouseWheel(MouseWheel e) {

	}

	void UINode::setClickListener(std::function<void()> listener) {
		if (listener) {
			_onClickListener = listener;
		}
	}

	bool UINode::intersects(int mx, int my, int x, int y, int width, int height) {
		int minX = x;
		int minY = y;
		int maxX = minX + width;
		int maxY = minY + height;
		return mx >= minX && mx <= maxX && my >= minY && my <= maxY;
	}

	bool UINode::intersects(int mx, int my, UINode * node) {
		return intersects(mx, my, node->getWorldX(), node->getWorldY(), node->getWidth(), node->getHeight());
	}

	bool UINode::intersects(const ButtonEvent& e, UINode * node) {
		return intersects(e.x, e.y, node);
	}

	void UINode::setLocation(int x, int y) {
		_x = x;
		_y = y;
	}
}