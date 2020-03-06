#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#define RMS_MIN_SAMPLES 480

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "AudioSink.h"
#include "AudioCapture.h"
#include "Wave.h"

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
	void GetAudioLevels(wxTimerEvent& event);

	wxTimer *timerAudioLevel;
	wxStaticText* textAudioLevel;
};

enum
{
	ID_New = 1,
	ID_StartAudio,
	ID_AudioLevelTimer
};

wxIMPLEMENT_APP(ASApp);

bool ASApp::OnInit()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	ASFrame* frame = new ASFrame("AudioShader", wxPoint(50, 50), wxSize(800, 680));
	frame->pApp = this;
	frame->Show(true);

	audioSink = new AudioSink();
	audioCapture = new AudioCapture();
	audioCapture->OpenDevice(audioSink, true);

	frame->SetStatusText(wxString::Format("%s - %d KHz %d-bit %d-Ch",
											audioCapture->GetDeviceName(),
											audioSink->GetSampleRate(),
											(audioSink->GetFrameSize() / 2)*8,
											audioSink->GetChannels()											
	));

	return true;
}

ASApp::~ASApp()
{
	if (audioCapture->isActive())
		audioCapture->Stop();

	if (audioSink != nullptr)
		delete audioSink;
	if (audioCapture != nullptr)
		delete audioCapture;

	_CrtDumpMemoryLeaks();
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
	

	Bind(wxEVT_MENU, &ASFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &ASFrame::OnAbout, this, wxID_ABOUT);

	Bind(wxEVT_MENU, &ASFrame::OnNew, this, ID_New);

	//
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* shaderSizer = new wxBoxSizer(wxHORIZONTAL);

	//audio panel
	wxPanel* audioPanel = new wxPanel(this, -1, wxDefaultPosition, { 800, 100 }, wxDOUBLE_BORDER);
	wxStaticText* staticText1 = new wxStaticText(audioPanel, wxID_ANY, "Audio Level:", {5, 5}, { 128, 16 });
	textAudioLevel = new wxStaticText(audioPanel, wxID_ANY, "0: dB", {5, 25}, { 128, 16 });
	wxButton* btnStartAudio = new wxButton(audioPanel, ID_StartAudio, "Start", { 5, 45 }, { 128, 20 });

	Bind(wxEVT_BUTTON, &ASFrame::OnAudioStart, this, ID_StartAudio);

	shaderSizer->Add( //Panel for GL Context
		new wxPanel(this, -1, wxDefaultPosition, { 800, 450 }, wxDOUBLE_BORDER),
		1, wxEXPAND | wxALL,
		5
	);
	shaderSizer->Add( //Panel for shader code
		new wxPanel(this, -1, wxDefaultPosition, { 380, 450 }, wxDOUBLE_BORDER),
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

	timerAudioLevel = new wxTimer(this, ID_AudioLevelTimer);
	Bind(wxEVT_TIMER, &ASFrame::GetAudioLevels, this, ID_AudioLevelTimer);
	timerAudioLevel->Start(100);
}

void ASFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void ASFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox(	"This is an audio visualizer that uses HLSL fragment shaders.",
					"About Audio Shader", wxOK | wxICON_INFORMATION);
}

void ASFrame::OnAudioStart(wxCommandEvent& event)
{
	pApp->audioCapture->StartCapture();

	/*Wave* waveFile = new Wave(pApp->audioSink->GetChannels(),
		pApp->audioSink->GetSampleRate(),
		pApp->audioSink->GetByteRate(),
		pApp->audioSink->GetFrameSize(),
		pApp->audioSink->GetBitRate()
	);
	waveFile->writeFile("test.wav");
	delete waveFile;*/
}

void ASFrame::GetAudioLevels(wxTimerEvent& event)
{
	uint32_t nFrames = pApp->audioSink->GetFramesCount();

	if (nFrames >= RMS_MIN_SAMPLES)
	{
		//get audio buffer
		uint8_t nChannels = pApp->audioSink->GetChannels();		
		uint32_t nDataSize = nFrames * pApp->audioSink->GetFrameSize();

		uint8_t* pAudioData = new uint8_t[nDataSize];
		pApp->audioSink->GetBuffer(pAudioData, nFrames);

		uint16_t nRMSSize = nFrames * nChannels;
		uint8_t nSampleSize = pApp->audioSink->GetFrameSize() / nChannels;
		double *pRMS = new double[nChannels];
		memset(pRMS, 0, sizeof(double) * nChannels);

		uint8_t* pData = pAudioData;
		
		for (int i = 0; i < nRMSSize; i++)
		{
			float *sample;
			sample = (float*)pData;
			pData += nSampleSize;

			uint8_t channel = i % nChannels;
			
			pRMS[channel] += *sample * *sample;
		}

		double dLevel = 0;

		for (int c = 0; c < nChannels; c++)
		{
			pRMS[c] /= (double)nFrames;
			pRMS[c] = sqrt(pRMS[c]);

			dLevel += pRMS[c];
		}

		dLevel /= nChannels;

		double dB = (dLevel > 0) ? (20 * log10(dLevel)) : (-1000000);

		textAudioLevel->SetLabel(wxString::Format("%.2f dB", dB));

		delete[] pAudioData;
		delete[] pRMS;
	}
}

void ASFrame::OnNew(wxCommandEvent& event)
{
	//TODO: IMPLEMENT
}


