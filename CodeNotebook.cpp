#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <fstream>
#include <string>
#include <sstream>

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
	wxString pageName = wxString::Format("Shader %d", (int)GetPageCount() + 1);
	codePage->SetName(pageName);

	AddPage(codePage, pageName, true);
}

void CodeNotebook::AddShader(wxString name, wxString content, wxString path)
{
	CodeEditor* codePage = new CodeEditor(this, content);
	codePage->SetName(name);
	codePage->SetPath(path);

	AddPage(codePage, name, true);
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

bool CodeNotebook::SaveShader(bool saveAs)
{
	CodeEditor* pCE = (CodeEditor*)GetPage(GetSelection());

	if (!pCE)
		return false;

	wxString path = "./shaders/";

	if (saveAs || pCE->GetPath() == "")
	{
		wxFileDialog saveFileDlg(this, _("Save shader"), path, "", "FRAG files (*.frag)|*.frag",
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if (saveFileDlg.ShowModal() == wxID_CANCEL)
			return false;

		path = saveFileDlg.GetPath();

		pCE->SetName(saveFileDlg.GetFilename());
		pCE->SetPath(path);
		SetPageText(GetSelection(), pCE->GetName());
	}
	else
	{
		path = pCE->GetPath();
	}

	std::ofstream outputStream(path.ToStdString());

	if ((outputStream.rdstate() & std::ofstream::failbit))
	{
		wxLogError("Failed saving to file '%s'.", path);
		return false;
	}
	
	outputStream << pCE->GetText().ToStdString();
	outputStream.flush();
	outputStream.close();

	return true;
}

bool CodeNotebook::OpenShader()
{
	wxFileDialog openFileDlg(this, _("Open .FRAG file"), "./shaders/", "", "FRAG files (*.frag)|*.frag",
								wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDlg.ShowModal() == wxID_CANCEL)
		return false;

	wxString path = openFileDlg.GetPath();

	std::ifstream inputFile(path.ToStdString());

	if (!inputFile.good())
	{
		wxLogError("Failed opening file '%s'.", path);
		return false;
	}

	std::string fileContent = "";
	std::stringstream inputStream;
	
	inputStream << inputFile.rdbuf();
	inputFile.close();

	fileContent = inputStream.str();

	AddShader(openFileDlg.GetFilename(), fileContent, path);

	return true;
}

