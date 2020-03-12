#include <wx/wxprec.h>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#define RMS_MIN_SAMPLES 512

#define MIN_SHADER_WIDTH 800
#define MIN_SHADER_HEIGHT 450

#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>
#include <string>

#include "AudioSink.h"
#include "AudioCapture.h"
#include "Wave.h"
#include "ShaderWindowGL.h"
#include "CodeEditor.h"
#include "CodeNotebook.h"

wxDEFINE_EVENT(EVENT_SEL_LINE, wxCommandEvent);

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
	void OnOpen(wxCommandEvent& event);
	void OnSaveAs(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

	void OnLineSelected(wxCommandEvent& event);

	void OnAudioStart(wxCommandEvent& event);
	void GetAudioLevels(wxTimerEvent& event);
	void ShowAudioInfo(wxTimerEvent& event);

	void OnCompileShader(wxCommandEvent& event);

	void CompileShader();

	wxTimer *timerAudioData;
	wxTimer* timerAudioPanel;
	wxStaticText* textAudioLevel;

	ShaderWindowGL* glShader;
	CodeNotebook* codeNotebook;
	wxStaticText* errorLog;
};

enum
{
	ID_New = 1,
	ID_SaveAs,
	ID_Save,
	ID_Open,
	ID_StartAudio,
	ID_AudioDataTimer,
	ID_AudioPanelTimer,
	ID_BtnCompile,
	ID_ErrorLogShader
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
	menuFile->Append(ID_New, _("&New\tCtrl-N"), _("New audio visualization."));
	menuFile->Append(ID_Open, _("&Open...\tCtrl-O"), _("Open a shader file from the disk"));
	menuFile->Append(ID_Save, _("&Save\tCtrl-S"));
	menuFile->Append(ID_SaveAs, _("Save As..."));
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuFile, _("&File"));
	menuBar->Append(menuHelp, _("&Help"));

	SetMenuBar(menuBar);

	CreateStatusBar();
	

	Bind(wxEVT_MENU, &ASFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &ASFrame::OnAbout, this, wxID_ABOUT);

	Bind(wxEVT_MENU, &ASFrame::OnNew, this, ID_New);
	Bind(wxEVT_MENU, &ASFrame::OnOpen, this, ID_Open);
	Bind(wxEVT_MENU, &ASFrame::OnSaveAs, this, ID_SaveAs);
	Bind(wxEVT_MENU, &ASFrame::OnSave, this, ID_Save);

	//
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* shaderSizer = new wxBoxSizer(wxHORIZONTAL);

	//audio panel
	wxPanel* audioPanel = new wxPanel(this, -1, wxDefaultPosition, { 800, 100 }, wxDOUBLE_BORDER);
	wxStaticText* staticText1 = new wxStaticText(audioPanel, wxID_ANY, "Audio Level:", {5, 5}, { 128, 16 });
	textAudioLevel = new wxStaticText(audioPanel, wxID_ANY, "0: dB", {5, 25}, { 128, 16 });
	wxButton* btnStartAudio = new wxButton(audioPanel, ID_StartAudio, "Start", { 5, 45 }, { 128, 20 });

	Bind(wxEVT_BUTTON, &ASFrame::OnAudioStart, this, ID_StartAudio);

	//Panel for GL Context
	wxPanel* shaderPanel = new wxPanel(this, -1, wxDefaultPosition, { MIN_SHADER_WIDTH, MIN_SHADER_HEIGHT }, wxSIMPLE_BORDER);

	int args[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };
	glShader = new ShaderWindowGL(shaderPanel, args);

	shaderSizer->Add( 
		shaderPanel,
		1, wxSHAPED | wxALL, //maintain aspect ratio
		5
	);
	
	//Panel for shader code notebook
	codeNotebook = new CodeNotebook(this, wxID_ANY, wxDefaultPosition, { MIN_SHADER_WIDTH, MIN_SHADER_HEIGHT });	

	wxBoxSizer* notebookSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* notebookBottomSizer = new wxBoxSizer(wxHORIZONTAL);
	notebookSizer->Add(codeNotebook, 1, wxEXPAND);

	//bottom of code notebook
	notebookBottomSizer->Add(new wxButton(this, ID_BtnCompile, "Compile && Run", wxDefaultPosition, { 128, 25 }),
		0, wxSHRINK | wxALL,
		0
	);
	
	errorLog = new wxStaticText(this, ID_ErrorLogShader, "", wxDefaultPosition, { 300, 25 });
	errorLog->SetForegroundColour(wxColor(0x0000FF));

	notebookBottomSizer->Add(
		errorLog,
		0, wxEXPAND | wxLEFT | wxRIGHT,
		5
	);

	notebookSizer->Add(notebookBottomSizer,
		0, wxSHRINK | wxALL,
		0
	);
	Bind(wxEVT_BUTTON, &ASFrame::OnCompileShader, this, ID_BtnCompile);

	shaderSizer->Add( 
		notebookSizer,
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

	timerAudioData = new wxTimer(this, ID_AudioDataTimer);
	Bind(wxEVT_TIMER, &ASFrame::GetAudioLevels, this, ID_AudioDataTimer);
	timerAudioData->Start(50);  //50ms looks pretty good. 100ms starts to look too unresponsive.

	timerAudioPanel = new wxTimer(this, ID_AudioPanelTimer);
	Bind(wxEVT_TIMER, &ASFrame::ShowAudioInfo, this, ID_AudioPanelTimer);
	timerAudioPanel->Start(100);

	//bind event to get events from selected lines in the code editor
	Bind(EVENT_SEL_LINE, &ASFrame::OnLineSelected, this, wxID_ANY);

	//load and comile template shader
	codeNotebook->NewShader();
	CompileShader();
}

void ASFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void ASFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox(	"This is an audio visualizer that uses GLSL fragment shaders.",
					"About Audio Shader", wxOK | wxICON_INFORMATION);
}

void ASFrame::OnLineSelected(wxCommandEvent& event)
{
	int line = event.GetInt();

	errorLog->SetLabel(codeNotebook->GetErrorOnLine(line));
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
		//pApp->audioSink->GetUnreadBuffer(pAudioData, nFrames);
		pApp->audioSink->GetLastFrames(pAudioData, 512);
		nFrames = 512;

		float* pAudioDataMono = new float[512];
		memset(pAudioDataMono, 0, sizeof(float) * 512);

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

			if (i < 1024)
			{
				if (channel == 0)
					pAudioDataMono[i / 2] = *sample;
				else
				{
					pAudioDataMono[i / 2] += *sample;
					pAudioDataMono[i / 2] /= 2.0;
				}
			}
		}

		//generate a 'texture' to sample the audio data from
		glShader->GenerateAudioSampler(pAudioDataMono, 512);

		double dLevel = 0;

		for (int c = 0; c < nChannels; c++)
		{
			pRMS[c] /= (double)nFrames;
			pRMS[c] = sqrt(pRMS[c]);

			dLevel += pRMS[c];
		}

		dLevel /= nChannels;

		//send the audio level to the shader
		glShader->SetAudioLevel(dLevel);

		//show the dBFS level in the audio panel
		/*double dB = (dLevel > 0) ? (20 * log10(dLevel)) : (-1000000);

		textAudioLevel->SetLabel(wxString::Format("%.2f dB Frames: %d", dB, nFrames));*/

		delete[] pAudioData;
		delete[] pRMS;
		delete[] pAudioDataMono;
	}
}

void ASFrame::ShowAudioInfo(wxTimerEvent& event)
{
	if (!pApp->audioCapture->isActive())
		return;

	//get average audio levels and display them on the Audio Panel
	uint64_t nNumFrames = pApp->audioSink->GetSampleRate() / 10;
	uint8_t nChannels = pApp->audioSink->GetChannels();
	uint64_t nNumSamples = nNumFrames * nChannels;

	float* pAudioData = new float[nNumSamples];
	pApp->audioSink->GetLastFrames(pAudioData, nNumFrames);

	double* fRMS = new double[nChannels];
	memset(fRMS, 0, sizeof(double) * nChannels);

	for (int i = 0; i < nNumSamples; i++)
	{
		int channel = i % nChannels;

		fRMS[channel] += (double)pAudioData[i] * pAudioData[i];
	}

	double dLevel = 0.0;

	for (int i = 0; i < nChannels; i++)
	{
		fRMS[i] /= nNumFrames;
		fRMS[i] = sqrt(fRMS[i]);

		dLevel += fRMS[i];
	}

	dLevel /= nChannels;

	//show the dBFS level in the audio panel
	double dB = (dLevel > 0.0) ? (20 * log10(dLevel)) : (-1000000);

	textAudioLevel->SetLabel(wxString::Format("%.2f dB", dB));

	delete[] fRMS;
	delete[] pAudioData;
}

void ASFrame::OnCompileShader(wxCommandEvent& event)
{
	CompileShader();
}

void ASFrame::CompileShader()
{
	codeNotebook->ClearErrors();

	if (!glShader->SetAndCompileShader(codeNotebook->GetCurrentCode()))
	{
		codeNotebook->ShowErrors(glShader->GetErrorLog());
		errorLog->SetLabel(codeNotebook->GetError(0));
	}
	else
	{
		errorLog->SetLabel("");
	}
}

void ASFrame::OnNew(wxCommandEvent& event)
{
	codeNotebook->NewShader();
}

void ASFrame::OnOpen(wxCommandEvent& event)
{
	codeNotebook->OpenShader();
}

void ASFrame::OnSaveAs(wxCommandEvent& event)
{
	codeNotebook->SaveShader(true);
}

void ASFrame::OnSave(wxCommandEvent& event)
{
	codeNotebook->SaveShader();
}


