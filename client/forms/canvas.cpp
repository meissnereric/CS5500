// Example application modified from the wxWidgets demo here:
// http://fossies.org/dox/wxWidgets-3.0.2/cube_8cpp_source.html

#include <GL/glut.h>
#include "canvas.h"
#include "main.h"

// control ids
enum
{
  SpinTimer = wxID_HIGHEST + 1
};

// ----------------------------------------------------------------------------
// TestGLCanvas
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(TestGLCanvas, wxGLCanvas) EVT_PAINT(TestGLCanvas::OnPaint)
  EVT_KEY_DOWN(TestGLCanvas::OnKeyDown)
  EVT_TIMER(SpinTimer, TestGLCanvas::OnSpinTimer) wxEND_EVENT_TABLE()

  TestGLCanvas::TestGLCanvas(wxWindow* parent, int* attribList)
  : wxGLCanvas(parent,
               wxID_ANY,
               attribList,
               wxDefaultPosition,
               wxDefaultSize,
               wxFULL_REPAINT_ON_RESIZE)
  , m_xangle(30.0)
  , m_yangle(30.0)
  , m_spinTimer(this, SpinTimer)
{
  // Init for the game loop.
  m_spinTimer.Start(16); // 16 milliseconds for 60 fps.
  chunk_manager = new ChunkManager();
}

TestGLCanvas::~TestGLCanvas()
{
  delete chunk_manager;
}

// Needed to use wxGetApp(). Usually you get the same result
// by using wxIMPLEMENT_APP(), but that can only be in main.
wxDECLARE_APP(MyApp);

void TestGLCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
  // This is required even though dc is not used otherwise.
  wxPaintDC dc(this);
  Render();
}

void TestGLCanvas::Resize()
{
  auto ClientSize = GetClientSize();
  glViewport(0, 0, ClientSize.x, ClientSize.y);
}

void TestGLCanvas::Spin(float xSpin, float ySpin)
{
  m_xangle += xSpin;
  m_yangle += ySpin;
}

void TestGLCanvas::OnKeyDown(wxKeyEvent& event)
{
  float angle = 5.0;

  switch (event.GetKeyCode())
  {
  case WXK_RIGHT:
    Spin(0.0, -angle);
    break;

  case WXK_LEFT:
    Spin(0.0, angle);
    break;

  case WXK_DOWN:
    Spin(-angle, 0.0);
    break;

  case WXK_UP:
    Spin(angle, 0.0);
    break;

  default:
    event.Skip();
    return;
  }
}

void TestGLCanvas::OnSpinTimer(wxTimerEvent& WXUNUSED(event))
{
  Update();
  Render();
}

void TestGLCanvas::Render()
{
  Spin(0.0, 4.0);

  // With perspective OpenGL graphics, the wxFULL_REPAINT_ON_RESIZE style
  // flag should always be set, because even making the canvas smaller should
  // be followed by a paint event that updates the entire canvas with new
  // viewport settings.
  Resize();

  // Clear screen.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render the graphics and swap the buffers.
  TestGLContext& context = wxGetApp().GetContext(this);
  context.DrawRotatedCube(m_xangle, m_yangle, 0.1f, -0.2f, 0.5f);
  context.DrawRotatedCube(m_xangle, m_yangle, -0.5f, 0.3f, 0.3f);
  SwapBuffers();
}

void TestGLCanvas::Update()
{
}
