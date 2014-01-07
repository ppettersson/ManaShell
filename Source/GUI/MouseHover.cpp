#include "MouseHover.h"

DEFINE_EVENT_TYPE(MOUSE_HOVERING)

MouseHover::MouseHover(wxWindow *_host, int _interval)
	: host(_host)
	, interval(_interval)
	, client(NULL)
{
}

void MouseHover::StartHoverDetect(wxWindow *win)
{
	win->Connect(wxEVT_ENTER_WINDOW, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnEnterWindow, NULL, this);	
	win->Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnLeaveWindow, NULL, this);	
	client = win;
}

void MouseHover::StopHoverDetect()
{
	if (client)
	{
		client->Disconnect(wxEVT_ENTER_WINDOW, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnEnterWindow);
		client->Disconnect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnLeaveWindow);
		client->Disconnect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_LEFT_UP, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_MIDDLE_DOWN, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_MIDDLE_UP, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_MIDDLE_DCLICK, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_RIGHT_UP, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_RIGHT_DCLICK, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_MOTION, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnMotion);
		client = NULL;
	}
}

void MouseHover::OnEnterWindow(wxMouseEvent &event)
{
	if (client)
	{
		client->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton, NULL, this);
		client->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton, NULL, this);
		client->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton, NULL, this);
		client->Connect(wxEVT_MIDDLE_DOWN, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton, NULL, this);
		client->Connect(wxEVT_MIDDLE_UP, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton, NULL, this);
		client->Connect(wxEVT_MIDDLE_DCLICK, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton, NULL, this);
		client->Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton, NULL, this);
		client->Connect(wxEVT_RIGHT_UP, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton, NULL, this);
		client->Connect(wxEVT_RIGHT_DCLICK, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton, NULL, this);
		client->Connect(wxEVT_MOTION, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnMotion, NULL, this);
	}
	event.Skip();
}

void MouseHover::OnLeaveWindow(wxMouseEvent &event)
{
	if (client)
	{
		client->Disconnect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_LEFT_UP, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_MIDDLE_DOWN, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_MIDDLE_UP, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_MIDDLE_DCLICK, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_RIGHT_UP, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_RIGHT_DCLICK, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnButton);
		client->Disconnect(wxEVT_MOTION, (wxObjectEventFunction)(wxEventFunction)(wxMouseEventFunction)&MouseHover::OnMotion);
	}
	wxTimer::Stop();
	event.Skip();
}

void MouseHover::OnButton(wxMouseEvent &event)
{
	wxTimer::Stop();
	event.Skip();
}

void MouseHover::OnMotion(wxMouseEvent &event)
{
	// Stop and restart timer.
	wxTimer::Stop();
	wxMouseState mouseState = wxGetMouseState();
	if (!mouseState.LeftIsDown() && !mouseState.MiddleIsDown() && !mouseState.RightIsDown())
		wxTimer::Start(interval, true);
	event.Skip();
}

void MouseHover::Notify()
{
	// Raise event.
	wxCommandEvent event(MOUSE_HOVERING);
	event.SetClientData(client);
	wxPostEvent(host, event);
}
