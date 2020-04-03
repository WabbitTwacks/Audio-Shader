#pragma once

#include <wx/glcanvas.h>
#include <gl/GLU.h>
#include <gl/glext.h>
#include <ctime>

class ShaderWindowGL : public wxGLCanvas
{
	void Render(wxTimerEvent& event);
	void Resized(wxSizeEvent& event);
	void Prep2DViewport(int topleftX, int topleftY, int bottomrightX, int bottomrightY);
	void* GetFuncAddress(const char* name);

	wxGLContext* m_context;
	wxWindow* m_parent;

	wxTimer* m_renderTimer;

	float m_time = 0; //time from shader start in seconds
	float m_audioLevel = 0.0;
	unsigned int m_audioSampler = 0;
	unsigned int m_testImage = 0;
	float* m_audioData = nullptr;

	char* m_fragShaderSource = nullptr;
	unsigned int m_vertexShader = 0;
	unsigned int m_fragmentShader = 0;
	unsigned int m_shaderProgram = 0;
	wxString m_errorLog = "";

	PFNGLCREATEPROGRAMPROC glCreateProgram;
	PFNGLCREATESHADERPROC glCreateShader;
	PFNGLSHADERSOURCEPROC glShaderSource;
	PFNGLCOMPILESHADERPROC glCompileShader;
	PFNGLGETSHADERIVPROC glGetShaderiv;
	PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
	PFNGLATTACHSHADERPROC glAttachShader;
	PFNGLLINKPROGRAMPROC glLinkProgram;
	PFNGLUSEPROGRAMPROC glUseProgram;
	PFNGLDELETESHADERPROC glDeleteShader;
	PFNGLGETPROGRAMIVPROC glGetProgramiv;
	PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
	PFNGLDETACHSHADERPROC glDetachShader;
	PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
	PFNGLUNIFORM1FPROC glUniformf;
	PFNGLUNIFORM2FPROC glUniform2f;
	PFNGLUNIFORM3FPROC glUniform3f;
	PFNGLUNIFORM4FPROC glUniform4f;
	PFNGLUNIFORM1IPROC glUniform1i;
	PFNGLUNIFORM2IPROC glUniform2i;
	PFNGLGETSTRINGIPROC glGetStringi;
	PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
	PFNGLACTIVETEXTUREPROC glActiveTexture;
	PFNGLUNIFORM1FVPROC glUniform1fv;

	std::string m_vertexShaderSource =
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aColor;\n"
		"layout (location = 2) in vec2 aTexCoord;\n\n"
		"out vec2 TexCoord;\n\n"
		"void main(){\n"
			"//gl_Position = vec4(aPos, 1.0);\n"
			"TexCoord = aTexCoord;\n"
		"}\0";

public:
	ShaderWindowGL(wxWindow* parent, int *args);
	virtual ~ShaderWindowGL();	

	int GetWidth();
	int GetHeight();

	void SetShaderSource(const char* shaderSource);
	void SetShaderSource(wxString);

	bool CompileShader();
	bool SetAndCompileShader(const char* shaderSource);
	bool SetAndCompileShader(wxString);

	void SetAudioLevel(float fLevel);

	bool GenerateAudioSampler(float* data, int size);

	wxString GetErrorLog() { return m_errorLog; }
};