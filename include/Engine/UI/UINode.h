#ifndef UINODE_H
#define UINODE_H

#include <string>
#include <vector>
#include <functional>

#include "Engine/Render/CursorType.h"
#include "Engine/Input/Input.h"
#include "Engine/Input/KeyListener.h"
#include "Engine/Input/MouseListener.h"

namespace NovaEngine {
	class TypeRenderer;

	struct ClipRect {
		int width;
		int height;
	};

	class UINode : public MouseListener, public KeyListener {
	protected:
		std::vector<UINode*> _childs;
	private:
		int _x = 0;
		int _y = 0;
		int _width = 0;
		int _height = 0;

		bool _focused = false;
		bool _clicked = false;
		bool _visible = true;
		bool _enabled = true;
		
		ClipRect _clipRect;
		bool _clipEnabled = false;

		std::vector<TypeRenderer*> _renderers;

		UINode* _parent;
		std::string _id;

		CursorType _cursor;

		std::function<void()> _onClickListener;
	public:
		UINode();
		virtual ~UINode();

		void setX(int x);
		void setY(int y);
		int getX() const;
		int getY() const;

		void setLocation(int x, int y);

		int getWorldX();
		int getWorldY();

		void setWidth(int width);
		void setHeight(int height);
		int getWidth() const;
		int getHeight() const;

		void setSize(int width, int height);

		void setClip(bool flag);
		void setClip(int width, int height);

		bool isClipEnabled();
		ClipRect& getClip();

		bool isZero() const;

		void setFocused(bool focused);
		void setClicked(bool clicked);
		void setVisible(bool visible);
		virtual void setEnabled(bool enabled);

		bool isFocused();
		bool isClicked();
		bool isVisible();
		bool isEnabled();

		void setParent(UINode* parent);
		UINode* getParent();

		void setId(std::string id);
		std::string getId();

		void addChild(UINode* node);
		void removeChild(UINode* node);
		int getChildCount();
		std::vector<UINode*>& getChilds();

		void setCursor(const CursorType& cursor);
		CursorType getCursor();

		void addRenderer(TypeRenderer* renderer, int layer);
		TypeRenderer* getRendrer(int index);
		int getRenderersCount() const;
		std::vector<TypeRenderer*>& getRenderers();

		template<typename T>
		inline T getRendererI(int index) {
			if (_basicRenderer == nullptr) {
				return nullptr;
			}
			return static_cast<T>(_renderers[index]);
		}

		virtual void onMouseMove(int x, int y, int dx, int dy) override;
		virtual void onMouseEvent(ButtonEvent e) override;
		virtual void onKeyEvent(KeyEvent e) override;
		virtual void onMouseWheel(MouseWheel e) override;

		void setClickListener(std::function<void()> listener);

		static bool intersects(int mx, int my, int x, int y, int width, int height);
		static bool intersects(int mx, int my, UINode* node);
		static bool intersects(const ButtonEvent& e, UINode* node);
	};
}

#endif // !UINODE_H