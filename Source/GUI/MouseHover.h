#ifndef MOUSE_HOVER_H
#define MOUSE_HOVER_H

#include <wx/wx.h>

// Define custom event MOUSE_HOVERING
BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_EVENT_TYPE(MOUSE_HOVERING, 0)
END_DECLARE_EVENT_TYPES()

// Class for handling detection of mouser hover in defined windows
class MouseHover: public wxTimer
{
public:
	MouseHover(wxWindow *_host, int _interval);

	void StartHoverDetect(wxWindow *win);
	void StopHoverDetect();

private:
	wxWindow *host;
	int interval;
	wxWindow *client;
	wxPoint position;

	void OnEnterWindow(wxMouseEvent& event);
	void OnLeaveWindow(wxMouseEvent& event);
	void OnButton(wxMouseEvent& event);
	void OnMotion(wxMouseEvent& event);

	virtual void Notify();
};

#endif // MOUSE_HOVER_H