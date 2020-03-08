//#include <string>

#include <wx/tokenzr.h>
#include <wx/regex.h>

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

void CodeNotebook::ShowErrors(wxString errorString)
{
	m_errorList.clear();
	m_mapLineToError.clear();

	wxStringTokenizer lines(errorString, '\n');

	while (lines.HasMoreTokens())
	{
		wxString error = lines.GetNextToken();
		m_errorList.push_back(error);

		wxRegEx reError;
		reError.Compile("ERROR: ?([0-9]+):([0-9]+):(.+)");

		if (reError.Matches(error, wxRE_EXTENDED))
		{
			long lineNum;
			long posNum;

			reError.GetMatch(error, 2).ToLong(&lineNum, 10);
			reError.GetMatch(error, 1).ToLong(&posNum, 10);

			m_mapLineToError[lineNum - 1] = m_errorList.size() - 1;

			CodeEditor* pCE = (CodeEditor*)GetPage(GetSelection());

			if (pCE)
			{
				pCE->MarkerAdd(lineNum-1, CE_MARKER_ERROR);

				//pCE->IndicatorFillRange(0, 80);
			}
		}
	}
}

wxString CodeNotebook::GetError(int n)
{
	if (m_errorList.size() >= n + 1)
		return m_errorList[n];

	return "";
}

wxString CodeNotebook::GetErrorOnLine(int l)
{
	if (m_mapLineToError.find(l) != m_mapLineToError.end())
	{
		return m_errorList[m_mapLineToError[l]];
	}

	return "";
}

void CodeNotebook::ClearErrors()
{
	CodeEditor* pCE = (CodeEditor*)GetPage(GetSelection());

	if (pCE)
	{
		pCE->MarkerDeleteAll(CE_MARKER_ERROR);
	}
}

