#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h>

#define CE_EDGE 128

wxDECLARE_EVENT(EVENT_SEL_LINE, wxCommandEvent);

enum
{
	CE_MARGIN_LINE_NUMBERS
};

enum
{
	CE_MARKER_ERROR
};

enum
{
	CE_IND_ERROR
};

class CodeEditor : public wxStyledTextCtrl
{
public:
	CodeEditor(wxWindow* parent, wxString text);

private:
	void MouseDown(wxStyledTextEvent& event);

};