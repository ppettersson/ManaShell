#ifndef REGISTERS_H
#define REGISTERS_H

#include "wx/wx.h"
#include "wx/grid.h"

class Registers : public wxGrid
{
public:
	Registers(wxWindow *parent);
	virtual ~Registers();

	void AddRegister(const wxString &name, const wxString &value);

private:
	unsigned	numRegisters;
};

#endif // REGISTERS_H
