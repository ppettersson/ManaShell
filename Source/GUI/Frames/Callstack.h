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
	void UpdateFrame(unsigned line);
	void ClearAllFrames();

	bool Load(const wxString &fileName);

	const wxString &CurrentFrame() const;
	const wxString &PreviousFrame() const;
	bool PopIfPreviousFrame(const wxString &description);

	size_t GetNumFrames() const										{ return frames.size(); }
	void ResetToFrame(unsigned frame);
	bool Sync(const wxString &description, const wxString &fileName, unsigned line);


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
