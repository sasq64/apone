#ifndef LIST_H
#define LIST_H

#include "../GL_Header.h"
#include "../rectangle.h"
#include <algorithm>

namespace grappix {

class VerticalLayout {
public:
	VerticalLayout() {}
	VerticalLayout(const Rectangle &screenArea, int visibleItems) : screenArea(screenArea) /*, visibleItems(visibleItems) */ {
		itemSize = screenArea / Rectangle(1, visibleItems);
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
		itemSize = screenArea / Rectangle(visibleItems, 1);
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

template <typename LAYOUT> class Base_List {
public:

	struct Renderer {
		virtual void render_item(Rectangle &rec, int y, uint32_t index, bool hilight) = 0;
	};

	Base_List() {}

	Base_List(std::function<void(Rectangle &rec, int y, uint32_t index, bool hilight)> renderFunc, const Rectangle &area, int visibleItems) : renderFunc(renderFunc), area(area), visibleItems(visibleItems), layout(area, visibleItems) {
	}

	Base_List(Renderer *renderer, const Rectangle &area, int visibleItems) : renderer(renderer), area(area), visibleItems(visibleItems), layout(area, visibleItems) {
	}

	void render() {
		//LOGD("POSITION %f", position);
		auto n = std::min(totalItems, visibleItems+1);
		float dummy;
		float p = modf(position, &dummy);
		int ip = (int)dummy;

		glEnable(GL_SCISSOR_TEST);
		glScissor(area.x, screen.height() - area.h - area.y, area.w, area.h);

		if(renderer != 0) {
			for(int i=0; i<n; i++) {
				auto rec = layout.layout((i - p) / (float)visibleItems);
				renderer->render_item(rec, i, i + ip, i + ip == selected_item);
			}
		} else {
			for(int i=0; i<n; i++) {
				auto rec = layout.layout((i + p) / (float)visibleItems);
				renderFunc(rec, i, i + position, i + ip == selected_item);
			}
		}

		glDisable(GL_SCISSOR_TEST);
	}

	void set_visible(int lines) {
		visibleItems = lines;
		layout = LAYOUT(area, visibleItems);
		select(selected_item);
	}

	void set_area(const Rectangle &r) {
		area = r;
		layout = LAYOUT(area, visibleItems);
		select(selected_item);
	}

	void set_total(uint32_t t) {
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

	int selected() { return selected_item; }

	void pagedown() { select(selected_item + visibleItems); }
	void pageup() { select(selected_item - visibleItems); }

	virtual bool on_key(grappix::Window::key k) {
		switch(k) {
		case Window::UP:
			select(selected()-1);
			break;
		case Window::DOWN:
			select(selected()+1);
			break;
		case Window::PAGEUP:
			pageup();
			break;
		case Window::PAGEDOWN:
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
	std::function<void(Rectangle &rec, int y, uint32_t index, bool hilight)> renderFunc;
	Rectangle area;
	int32_t visibleItems;
	LAYOUT layout;
	double position = 0;
	int32_t selected_item = 0;
	int32_t totalItems = 0;
};


typedef Base_List<VerticalLayout> VerticalList;
typedef Base_List<HorizontalLayout> HorizontalList;

}

#endif // LIST_H
