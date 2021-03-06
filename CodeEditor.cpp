#include "CodeEditor.h"

CodeEditor::CodeEditor(wxWindow* parent, wxString text) :
	wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, {700, 500})
{
	StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(0x202020));
	StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(0xDDDDDD));
	StyleSetBold(wxSTC_STYLE_DEFAULT, true);
	SetSelBackground(true, wxColor(0x505050));
	SetCaretForeground(wxColor(0xDDDDDD));
	SetCaretWidth(3);
	StyleClearAll(); //clear all except the default

	SetLexer(wxSTC_LEX_CPP);

	//SendMsg(wxSTC_STYLESET, )
	
	SetMarginWidth(CE_MARGIN_LINE_NUMBERS, 50);
	StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(75, 75, 75));
	StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(220, 220, 220));
	SetMarginType(CE_MARGIN_LINE_NUMBERS, wxSTC_MARGIN_NUMBER);

	SetWrapMode(wxSTC_WRAP_NONE);	

	StyleSetBold(wxSTC_C_WORD, true);
	StyleSetBold(wxSTC_C_WORD2, true);

	StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(165, 105, 0));
	StyleSetForeground(wxSTC_C_IDENTIFIER, wxColour(0x88D8B0));
	StyleSetForeground(wxSTC_C_NUMBER, wxColour(0xADEEFF));
	StyleSetForeground(wxSTC_C_WORD, wxColour(150, 50, 50));
	//StyleSetForeground(wxSTC_C_WORD2, wxColour(150, 150, 0));
	StyleSetForeground(wxSTC_C_WORD2, wxColour(0x5cccff));
	StyleSetForeground(wxSTC_C_COMMENT, wxColour(150, 150, 150));
	StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(150, 150, 150));

	SetKeyWords(0, wxT("return for while break continue if else in out uniform"));
	SetKeyWords(1, wxT("const int uint float bool void vec2 vec3 vec4 ivec2 ivec3 ivec4 "
						"bvec2 bvec3 bvec4 uvec2 uvec3 uvec4 dvec2 dvec3 dvec4 "
						"mat2 mat3 mat4 sampler1D sampler2D sampler3D samplerCube"));

	MarkerSetBackground(CE_MARKER_ERROR, wxColor(0x0000DD));

	IndicatorSetStyle(CE_IND_ERROR, wxSTC_INDIC_SQUIGGLELOW);
	IndicatorSetForeground(CE_IND_ERROR, wxColor(0x0000DD));

	SetEdgeColumn(CE_EDGE);
	SetEdgeMode(wxSTC_EDGE_LINE);

	SetText(text);

	Bind(wxEVT_STC_UPDATEUI, &CodeEditor::MouseDown, this, wxID_ANY);
}

void CodeEditor::MouseDown(wxStyledTextEvent& event)
{
	if (event.GetUpdated() & wxSTC_UPDATE_SELECTION)
	{
		wxCommandEvent e(EVENT_SEL_LINE, GetId());
		e.SetEventObject(this);
		e.SetInt(GetCurrentLine());

		ProcessWindowEvent(e);		
	}	

	event.Skip();
}
