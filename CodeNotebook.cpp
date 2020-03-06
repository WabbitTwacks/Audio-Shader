//#include <string>

#include "CodeNotebook.h"

CodeNotebook::CodeNotebook(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) :
	wxNotebook(parent, id, pos, size)
{
	SetBackgroundColour(wxColor(100, 100, 100));

	NewShader();
}

void CodeNotebook::NewShader()
{	
	CodeEditor* codePage = new CodeEditor(this, m_strStartCode);
	AddPage(codePage, "Shader 1");
}

wxString CodeNotebook::GetCurrentCode()
{
	CodeEditor* pCE = (CodeEditor*)GetPage(GetSelection());

	if (pCE)
	{
		return pCE->GetText();
	}

	return "";
}
