#pragma once

#include <wx/glcanvas.h>
#include <gl/GLU.h>
#include <gl/glext.h>
#include <ctime>

class ShaderWindowGL : public wxGLCanvas
{
	void Render(wxTimerEvent& event);
	void* GetFuncAddress(const char* name);

	wxGLContext* m_context;
	wxWindow* m_parent;

	wxTimer* m_renderTimer;

	float m_time = 0; //time from shader start in seconds

	char* m_fragShaderSource = nullptr;
	unsigned int m_fragmentShader = 0;
	unsigned int m_shaderProgram = 0;

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
	PFNGLGETSTRINGIPROC glGetStringi;

public:
	ShaderWindowGL(wxWindow* parent, int *args);
	virtual ~ShaderWindowGL();	

	void Resized(wxSizeEvent& event);

	int GetWidth();
	int GetHeight();

	void SetShaderSource(const char* shaderSource);
	bool CompileShader();
	bool SetAndCompileShader(const char* shaderSource);

	void Prep2DViewport(int topleftX, int topleftY, int bottomrightX, int bottomrightY);
};