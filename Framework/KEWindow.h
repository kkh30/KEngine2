#ifndef _KE_WINDOW_H__
#define _KE_WINDOW_H__
#include "WSIWindow\WSIWindow.h"
#include "KEModule.h"
#include <string>

static const char *type[]{ "up  ", "down", "move" };  // Action types for mouse, keyboard and touch-screen.
struct KEWindowDesc
{
	std::string title = "";
	int x, y, width, height = 0;
};
class KEWindow final : public WSIWindow, public Module<KEWindow>
{
public:
	KEWindow(const KEWindowDesc& p_desc);
	KEWindow(KEWindowDesc&& p_desc);

	~KEWindow();

	void OnMouseEvent(eAction action, int16_t x, int16_t y, uint8_t btn) { printf("%s %d x %d Btn:%d\n", type[action], x, y, btn); }
	void OnTouchEvent(eAction action, float x, float y, uint8_t id) { printf("Touch: %s %f x %f id:%d\n", type[action], x, y, id); }
	void OnKeyEvent(eAction action, eKeycode keycode) { printf("Key: %s keycode:%d\n", type[action], keycode); }
	void OnTextEvent(const char *str) { printf("Text: %s\n", str); }
	void OnMoveEvent(int16_t x, int16_t y) { printf("Window Move: x=%d y=%d\n", x, y); }
	void OnFocusEvent(bool hasFocus) { printf("Focus: %s\n", hasFocus ? "True" : "False"); }
	void OnResizeEvent(uint16_t width, uint16_t height) { printf("Window Resize: width=%4d height=%4d\n", width, height); }
	void OnCloseEvent() { printf("Window Closing.\n"); }
	virtual void OnStartUp() override;

private:
	KEWindowDesc m_desc = {};
};



#endif