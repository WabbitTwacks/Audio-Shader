#include "wx/wx.h"
#include "ShaderWindowGL.h"

ShaderWindowGL::ShaderWindowGL(wxWindow* parent, int *args)
	:wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    m_context = new wxGLContext(this);
    m_parent = parent;

    //get OpenGL functions
    SetCurrent(*m_context);

    glGetStringi = (PFNGLGETSTRINGIPROC)wglGetProcAddress("glGetStringi");

    int n = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);    

    OutputDebugStringA("GL_EXTENSIONS:\n");
    for (int i = 0; i < n; i++)
    {
        OutputDebugStringA((LPCSTR)glGetStringi(GL_EXTENSIONS, i));
        OutputDebugStringA("\n");
    }      

    glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
    glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
    glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
    glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
    glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
    glUniformf = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
    glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
    glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
    glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
    glUniform2i = (PFNGLUNIFORM2IPROC)wglGetProcAddress("glUniform2i");

    //bind Resized and Render Timer
    parent->Bind(wxEVT_SIZE, &ShaderWindowGL::Resized, this, wxID_ANY);

    m_renderTimer = new wxTimer(this);
    Bind(wxEVT_TIMER, &ShaderWindowGL::Render, this, wxID_ANY);
    m_renderTimer->Start(16.666666); //60 FPS

    //SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

ShaderWindowGL::~ShaderWindowGL()
{
    glDetachShader(m_shaderProgram, m_fragmentShader);
    glDeleteShader(m_fragmentShader);

    delete m_context;
    delete m_fragShaderSource;
}

void ShaderWindowGL::Render(wxTimerEvent& event)
{
    if (!IsShown())
        return;

    clock_t deltaTime = clock()/(float)CLOCKS_PER_SEC - m_time;
    m_time = clock() / (float)CLOCKS_PER_SEC;

    SetCurrent(*m_context);
    wxClientDC dc(this);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glViewport(0, 0, (GLint)GetSize().x, (GLint)GetSize().y);
    Prep2DViewport(0, 0, GetWidth(), GetHeight());

    //shader uniforms
    int iTime = glGetUniformLocation(m_shaderProgram, "iTime");
    int iResolution = glGetUniformLocation(m_shaderProgram, "iResolution");
    int iAudioLevelAvg = glGetUniformLocation(m_shaderProgram, "iAudioLevelAvg");
    glUniformf(iTime, m_time);
    glUniform2f(iResolution, GetWidth(), GetHeight());
    glUniformf(iAudioLevelAvg, m_audioLevel);

    //background
    glColor4f(0.4, 0.1, 0.9, 1);
    glBegin(GL_QUADS);
        glVertex3f(0, 0, 0);
        glVertex3f(GetWidth(), 0, 0);
        glVertex3f(GetWidth(), GetHeight(), 0);
        glVertex3f(0, GetHeight(), 0);
    glEnd();

    glFlush();
    SwapBuffers();
}

void* ShaderWindowGL::GetFuncAddress(const char* name)
{
    void* p = (void*)wglGetProcAddress(name);

    if (p == 0 ||
        (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
        (p == (void*)-1))
    { //fallback
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void*)GetProcAddress(module, name);
    }

    return p;
}

void ShaderWindowGL::Resized(wxSizeEvent& event)
{
    SetSize({m_parent->GetSize().x, m_parent->GetSize().y});
    Refresh();
}

int ShaderWindowGL::GetWidth()
{
    return GetSize().x;
}

int ShaderWindowGL::GetHeight()
{
    return GetSize().y;
}

void ShaderWindowGL::SetShaderSource(const char* shaderSource)
{
    delete m_fragShaderSource;

    unsigned int iSize = strlen(shaderSource) + 1;

    m_fragShaderSource = new char[iSize];
    memcpy_s(m_fragShaderSource, iSize, shaderSource, iSize);
}

void ShaderWindowGL::SetShaderSource(wxString shaderSource)
{
    delete m_fragShaderSource;

    unsigned int iSize = shaderSource.length() + 1;

    m_fragShaderSource = new char[iSize];

    memcpy_s(m_fragShaderSource, iSize, (const char*)shaderSource.mbc_str(), iSize);
}

bool ShaderWindowGL::CompileShader()
{
    glDetachShader(m_shaderProgram, m_fragmentShader);
    glDeleteShader(m_fragmentShader);

    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &m_fragShaderSource, NULL);
    glCompileShader(m_fragmentShader);

    int success;
    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        int length = 0;
        glGetShaderiv(m_fragmentShader, GL_INFO_LOG_LENGTH, &length);
        char* errorLog = new char[length];
        glGetShaderInfoLog(m_fragmentShader, length, NULL, errorLog);

        m_errorLog = errorLog;

        //return to somewhere
        OutputDebugString(L"ERROR::SHADER COMPILATION\n");
        OutputDebugStringA(errorLog);
        OutputDebugString(L"\n");

        delete[] errorLog;

        return false;
    }

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, m_fragmentShader);
    glLinkProgram(m_shaderProgram);

    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        int length = 0;
        glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &length);
        char* errorLog = new char[length];
        glGetProgramInfoLog(m_shaderProgram, 512, NULL, errorLog);

        m_errorLog = errorLog;

        //return to somewhere
        OutputDebugString(L"ERROR::SHADER LINKING\n");
        OutputDebugStringA(errorLog);
        OutputDebugString(L"\n");

        delete[] errorLog;

        return false;
    }

    glUseProgram(m_shaderProgram);

    return true;
}

bool ShaderWindowGL::SetAndCompileShader(const char* shaderSource)
{
    SetShaderSource(shaderSource);
    return CompileShader();
}

bool ShaderWindowGL::SetAndCompileShader(wxString shaderSource)
{
    SetShaderSource(shaderSource);
    return CompileShader();
}

void ShaderWindowGL::SetAudioLevel(float fLevel)
{
    if (fLevel > 1.0)
        fLevel = 1.0;

    m_audioLevel = fLevel;
}

void ShaderWindowGL::Prep2DViewport(int topleftX, int topleftY, int bottomrightX, int bottomrightY)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(topleftX, topleftY, bottomrightX - topleftX, bottomrightY - topleftY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(topleftX, bottomrightX, bottomrightY, topleftY);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
