#pragma once

#include <wx/notebook.h>
#include <vector>

#include "CodeEditor.h"

class CodeNotebook : public wxNotebook
{
public:
	CodeNotebook(wxWindow *parent, wxWindowID id = -1, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);

	void NewShader();

	wxString GetCurrentCode();

private:
	 
	std::string m_strStartCode =
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{ \n"
		"	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\n";
};