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

	void SetName(wxString name) { m_strName = name; }
	wxString GetName() { return m_strName; }
	void SetPath(wxString path) { m_filePath = path; }
	wxString GetPath() { return m_filePath; }

private:
	void MouseDown(wxStyledTextEvent& event);
	wxString m_strName = "";
	wxString m_filePath = "";
};