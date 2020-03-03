#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include "AudioSink.h"
#include "AudioCapture.h"

class ASApp : public wxApp
{
public:
	virtual ~ASApp();
	virtual bool OnInit();

	AudioSink* audioSink = nullptr;
	AudioCapture* audioCapture = nullptr;
};

class ASFrame : public wxFrame
{
public:
	ASFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

	ASApp* pApp = nullptr;

private:
	void OnNew(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

	void OnAudioStart(wxCommandEvent& event);	
};

enum
{
	ID_New = 1,
	ID_StartAudio
};

wxIMPLEMENT_APP(ASApp);

bool ASApp::OnInit()
{
	ASFrame* frame = new ASFrame("AudioShader", wxPoint(50, 50), wxSize(800, 680));
	frame->pApp = this;
	frame->Show(true);

	audioSink = new AudioSink();
	audioCapture = new AudioCapture();

	//audioCapture->GetStream(audioSink);

	return true;
}

ASApp::~ASApp()
{
	if (audioSink != nullptr)
		delete audioSink;
	if (audioCapture != nullptr)
		delete audioCapture;
}

ASFrame::ASFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(NULL, wxID_ANY, title, pos, size)
{
	//Menu setup
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_New, "&New...\tCtrl-N", "New audio visualization.");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Audio Shader vAlpha");

	Bind(wxEVT_MENU, &ASFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &ASFrame::OnAbout, this, wxID_ABOUT);

	Bind(wxEVT_MENU, &ASFrame::OnNew, this, ID_New);

	//
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* shaderSizer = new wxBoxSizer(wxHORIZONTAL);

	wxPanel* audioPanel = new wxPanel(this, -1, wxDefaultPosition, { 800, 100 }, wxDOUBLE_BORDER);
	wxStaticText* staticText1 = new wxStaticText(audioPanel, wxID_ANY, "Audio Level:", {5, 5}, { 128, 16 });
	wxStaticText* textAudioLevel = new wxStaticText(audioPanel, wxID_ANY, "0: dB", {5, 20}, { 128, 16 });
	wxButton* btnStartAudio = new wxButton(audioPanel, ID_StartAudio, "Start", { 5, 40 }, { 128, 20 });

	Bind(wxEVT_BUTTON, &ASFrame::OnAudioStart, this, ID_StartAudio);

	shaderSizer->Add( //Panel for GL Context
		new wxPanel(this, -1, wxDefaultPosition, { 380, 400 }, wxDOUBLE_BORDER),
		1, wxEXPAND | wxALL,
		5
	);
	shaderSizer->Add( //Panel for shader code
		new wxPanel(this, -1, wxDefaultPosition, { 380, 400 }, wxDOUBLE_BORDER),
		1, wxEXPAND | wxALL,
		5
	);

	//bottomsizer->AddStretchSpacer();
	bottomSizer->Add(shaderSizer, 1, wxEXPAND | wxALL, 5);
	bottomSizer->Add( //Panel for audio UI
		audioPanel,
		0, //vertically stretchable
		wxEXPAND | wxALL, //horizontal stretch | border
		5	//border width
	);
	
	SetSizerAndFit(bottomSizer);
}

void ASFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void ASFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox(	"This is an audio visualizer that uses HLSL fragment shaders.",
					"About Hello World", wxOK | wxICON_INFORMATION);
}

void ASFrame::OnAudioStart(wxCommandEvent& event)
{
	pApp->audioCapture->StartCapture();
}

void ASFrame::OnNew(wxCommandEvent& event)
{
	//TODO: IMPLEMENT
}


