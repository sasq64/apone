#ifndef LIST_H
#define LIST_H

#include "../GL_Header.h"
#include "../rectangle.h"
#include "renderable.h"
#include <math.h>
#include <algorithm>

namespace grappix {

class VerticalLayout {
public:
	VerticalLayout() {}
	VerticalLayout(const Rectangle &screenArea, int visibleItems) : screenArea(screenArea) /*, visibleItems(visibleItems) */ {
		itemSize = visibleItems <= 0 ? screenArea : screenArea / Rectangle(1, visibleItems);
	}

	Rectangle layout(double position) {
		Rectangle res = itemSize;
		res.x = screenArea.x;
		res.y = (screenArea.h) * position + screenArea.y;
		return res;
	}
private:
	Rectangle screenArea;
	//int visibleItems;
	Rectangle itemSize;
};

class HorizontalLayout {
public:
	HorizontalLayout() {}
	HorizontalLayout(const Rectangle &screenArea, int visibleItems) : screenArea(screenArea) /*, visibleItems(visibleItems) */ {
		itemSize = visibleItems <= 0 ? screenArea : screenArea / Rectangle(visibleItems, 1);
	}

	Rectangle layout(double position) {
		Rectangle res = itemSize;
		res.y = screenArea.y;
		res.x = (screenArea.w) * position + screenArea.x;
		return res;
	}
private:
	Rectangle screenArea;
	//int visibleItems;
	Rectangle itemSize;
};


template <typename LAYOUT> class BaseList : public Renderable {
public:

	using RenderItemFn = std::function<void(Rectangle &, int, uint32_t, bool)>;

	struct Renderer {
		virtual void renderItem(Rectangle &rec, int y, uint32_t index, bool hilight) = 0;
	};

	BaseList() {}

	BaseList(Renderer *renderer, const Rectangle &area, int visibleItems) : renderer(renderer), area(area), visibleItems(visibleItems), layout(area, visibleItems) {
	}

	BaseList(const Rectangle &area, int visibleItems, RenderItemFn fn) : renderer(nullptr), area(area), visibleItems(visibleItems), layout(area, visibleItems), renderFunc(fn) {
	}

	void render(std::shared_ptr<RenderTarget> target, uint32_t delta) override {
		//LOGD("POSITION %f", position);
		auto n = visibleItems+1;
		float dummy;
		float p = modff(position, &dummy);
		int ip = (int)dummy;

		glEnable(GL_SCISSOR_TEST);
		glScissor(area.x, screen.height() - area.h - area.y, area.w, area.h);

		if(renderer != 0) {
			for(int i=0; i<n; i++) {
				if(i + ip >= totalItems)
					break;
				auto rec = layout.layout((i - p) / (float)visibleItems);
				renderer->renderItem(rec, i, i + ip, i + ip == selected_item);
			}
		} else {
			for(int i=0; i<n; i++) {
				if(i + ip >= totalItems)
					break;
				auto rec = layout.layout((i - p) / (float)visibleItems);
				renderFunc(rec, i, i + ip, i + ip == selected_item);
			}
		}

		glDisable(GL_SCISSOR_TEST);
	}

	void setVisible(int lines) {
		visibleItems = lines;
		layout = LAYOUT(area, visibleItems);
		select(selected_item);
	}

	void setArea(const Rectangle &r) {
		area = r;
		layout = LAYOUT(area, visibleItems);
		select(selected_item);
	}

	void setTotal(uint32_t t) {
		totalItems = t;
		select(selected_item);
	}

	void select(uint32_t index) {
		selected_item = index;
		if(selected_item < 0)
			selected_item = 0;
		else if(selected_item >= totalItems)
			selected_item = totalItems-1;
		double p = position;
		if(selected_item-2 < p) {
			p = selected_item-2;
			if(p < 0) p = 0;
		}
		if(selected_item+2 >= p+visibleItems) {
			p = (int)(selected_item+2-visibleItems+1);
			if(p > totalItems-visibleItems)
				p = totalItems-visibleItems;
		}

		if(totalItems <= visibleItems)
			p = 0;

		if(p != position) {
			tween::Tween::make().to(position, p).seconds(0.2);
		}
	}

	void scroll(double x) {
		x += position;
		if(x != position && x > 0) {
			tween::Tween::make().to(position, x).seconds(0.2);
		}
	}

	int selected() { return selected_item; }

	void pagedown() { select(selected_item + visibleItems); }
	void pageup() { select(selected_item - visibleItems); }

	virtual bool onKey(int32_t k) {
		switch(k) {
		case keycodes::UP:
			select(selected()-1);
			break;
		case keycodes::DOWN:
			select(selected()+1);
			break;
		case keycodes::PAGEUP:
			pageup();
			break;
		case keycodes::PAGEDOWN:
			pagedown();
			break;
		default:
			return false;
		}
		return true;
	}

	int size() { return totalItems; }
	
private:

	Renderer *renderer;
	RenderItemFn renderFunc;
	Rectangle area;
	int32_t visibleItems;
	LAYOUT layout;
	double position = 0;
	int32_t selected_item = 0;
	int32_t totalItems = 0;
};


typedef BaseList<VerticalLayout> VerticalList;
typedef BaseList<HorizontalLayout> HorizontalList;

}

#endif // LIST_H
