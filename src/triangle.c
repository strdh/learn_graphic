#include <windows.h>
#include <glad/glad.h>
#include <stdio.h>

// Shader source code
const char* vertexShaderSource = R"(
  #version 460 core
  layout (location = 0) in vec3 aPos;
  layout (location = 1) in vec3 aColor;
  out vec3 vertexColor;
  
  void main() {
    gl_Position = vec4(aPos, 1.0);
    vertexColor = aColor;
  }
)";

const char* fragmentShaderSource = R"(
  #version 460 core
  in vec3 vertexColor;
  out vec4 FragColor;
  
  void main() {
    FragColor = vec4(vertexColor, 1.0);
  }
)";

// Global variables for shader program and buffers
GLuint shaderProgram;
GLuint VAO, VBO, colorVBO;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  if (uMsg == WM_CLOSE || uMsg == WM_DESTROY) {
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CheckOpenGLVersion() {
  const GLubyte* version = glGetString(GL_VERSION);
  const GLubyte* renderer = glGetString(GL_RENDERER);
  const GLubyte* vendor = glGetString(GL_VENDOR);
  const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

  GLint majorVersion, minorVersion;
  glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

  char infoMessage[1024];
  sprintf(infoMessage,
      "OpenGL Version: %s\n"
      "GLSL Version: %s\n"
      "Vendor: %s\n"
      "Renderer: %s\n"
      "Version (numeric): %d.%d",
      version, glslVersion, vendor, renderer,
      majorVersion, minorVersion);

  MessageBoxA(NULL, infoMessage, "OpenGL Information", MB_OK | MB_ICONINFORMATION);

  if (majorVersion < 4) {
    MessageBoxA(NULL, "This program requires OpenGL 4.0 or higher", "Version Error", MB_OK | MB_ICONERROR);
    exit(-1);
  }
}

GLuint CompileShader(const char* source, GLenum type) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  // Check compilation status
  GLint success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    MessageBoxA(NULL, infoLog, "Shader Compilation Error", MB_OK | MB_ICONERROR);
    exit(-1);
  }
  return shader;
}

void InitShaders() {
  // Compile vertex and fragment shaders
  GLuint vertexShader = CompileShader(vertexShaderSource, GL_VERTEX_SHADER);
  GLuint fragmentShader = CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

  // Create and link shader program
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // Check linking status
  GLint success;
  char infoLog[512];
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    MessageBoxA(NULL, infoLog, "Shader Program Linking Error", MB_OK | MB_ICONERROR);
    exit(-1);
  }

  // Clean up shaders (they're now part of the program)
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void InitBuffers() {
  // Define vertex positions and colors
  float vertices[] = {
       0.0f,  0.5f, 0.0f,  // Top vertex
      -0.5f, -0.5f, 0.0f,  // Bottom left vertex
       0.5f, -0.5f, 0.0f   // Bottom right vertex
  };

  float colors[] = {
      1.0f, 0.0f, 0.0f,  // Red color for top vertex
      0.0f, 1.0f, 0.0f,  // Green color for bottom left vertex
      0.0f, 0.0f, 1.0f   // Blue color for bottom right vertex
  };

  // Create and bind VAO
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // Create and set up vertex buffer
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // Create and set up color buffer
  glGenBuffers(1, &colorVBO);
  glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
}

void InitOpenGL(HDC hdc) {
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR), 1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    PFD_TYPE_RGBA, 32,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0 
  };

  int pixelFormat = ChoosePixelFormat(hdc, &pfd);
  SetPixelFormat(hdc, pixelFormat, &pfd);

  HGLRC hglrc = wglCreateContext(hdc);
  wglMakeCurrent(hdc, hglrc);

  if (!gladLoadGL()) {
    MessageBox(NULL, "Failed to initialize OpenGL context", "Error", MB_OK);
    exit(-1);
  }

  CheckOpenGLVersion();
  InitShaders();
  InitBuffers();
}

void RenderTriangle() {
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Use shader program and draw
  glUseProgram(shaderProgram);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  // Register window class
  WNDCLASS wc = { 0 };
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = "OpenGLWindowClass";
  wc.style = CS_OWNDC;
  RegisterClass(&wc);

  // Create window
  HWND hwnd = CreateWindowEx(
      0,
      wc.lpszClassName,
      "Tipis-tipis grafika",
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT, CW_USEDEFAULT,
      800, 600,
      NULL, NULL,
      hInstance,
      NULL
  );

  HDC hdc = GetDC(hwnd);
  InitOpenGL(hdc);

  // Main message loop
  MSG msg;
  while (1) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) break;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    RenderTriangle();
    SwapBuffers(hdc);
  }

  // Cleanup
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &colorVBO);
  glDeleteProgram(shaderProgram);

  HGLRC hglrc = wglGetCurrentContext();
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hglrc);
  ReleaseDC(hwnd, hdc);
  DestroyWindow(hwnd);

  return 0;
}
