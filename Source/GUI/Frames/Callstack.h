#ifndef CALL_STACK_H
#define CALL_STACK_H

#include "wx/wx.h"
#include <vector>

class MainFrame;

class Callstack : public wxListBox
{
public:
	Callstack(MainFrame *parent);
	virtual ~Callstack();

	void PushFrame(const wxString &description, const wxString &fileName, unsigned line);
	void PopFrame();

	bool Load(const wxString &fileName);


private:
	struct Frame
	{
		wxString		description,
						fileName;
		unsigned		line;


		Frame()
			: line(0)
		{
		}

		Frame(const wxString &d, const wxString &f, unsigned l)
			: description(d)
			, fileName(f)
			, line(l)
		{
		}
	};

	MainFrame			*host;
	std::vector<Frame>	frames;


	void LoadLine(const wxString &line);

	void OnDClick(wxCommandEvent &event);


	wxDECLARE_EVENT_TABLE();
};

#endif // CALL_STACK_H
