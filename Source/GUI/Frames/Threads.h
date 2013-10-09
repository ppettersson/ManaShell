#ifndef THREADS_H
#define THREADS_H

#include "wx/wx.h"
#include "wx/grid.h"

class Threads : public wxGrid
{
public:
	Threads(wxWindow *parent);
	virtual ~Threads();

	void AddThread(const wxString &id, const wxString &name);


private:
	unsigned	numThreads,
				activeThread;

	void UpdateActiveThread();
};

#endif // THREADS_H
