#pragma once

#include <wx/glcanvas.h>
#include <gl/GLU.h>
#include <ctime>

class ShaderWindowGL : public wxGLCanvas
{
	void Render(wxTimerEvent& event);

	wxGLContext* m_context;
	wxWindow* m_parent;

	wxTimer* m_renderTimer;

	float m_time = 0; //time from shader start in seconds

public:
	ShaderWindowGL(wxWindow* parent, int *args);
	virtual ~ShaderWindowGL();	

	void Resized(wxSizeEvent& event);

	int GetWidth();
	int GetHeight();

	void Prep2DViewport(int topleftX, int topleftY, int bottomrightX, int bottomrightY);
};