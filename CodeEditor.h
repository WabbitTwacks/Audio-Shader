#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h>

enum
{
	MARGIN_LINE_NUMBERS
};

class CodeEditor : public wxStyledTextCtrl
{
public:
	CodeEditor(wxWindow* parent, wxString text);

};