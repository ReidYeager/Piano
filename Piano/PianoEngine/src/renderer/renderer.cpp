
#include "defines.h"
#include "logger.h"

#include "renderer/renderer.h"
#include "platform/filesystem.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <vector>
#include <map>

vec2I windowExtents;

std::map<char, Piano::TextGlyph> fontGlyphs;

const glm::mat4 identityMatrix = glm::mat4(1.0f);

Piano::Material noteMaterial;
Piano::Material textMaterial;
glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);
glm::mat4 screenspaceMatrix = glm::mat4(1.0f);

std::vector<Piano::note> whiteKeyTransforms;
std::vector<Piano::note> blackKeyTransforms;

//const vec3 whiteColor = {0.87f, 0.79f, 0.05f}; // yellow
const vec3 whiteColor = {1.0f, 1.0f, 1.0f};
const vec3 blackColor = {0.0f, 0.0f, 1.0f};
const vec3 octaveColor = {0.5f, 0.5f, 0.5f};
const f32 octaveWidth = 0.2f;

void(*OctaveLineRender)();

struct textRenderData
{
  vec4 transform;
  u32 textureID;
  vec3 color;
};

std::vector<textRenderData> screenspaceText;
std::vector<textRenderData> worldspaceText;

void DoNothing() {}
void RenderOctaveLines();

//=========================
// Init & Shutdown
//=========================

u32 LoadShader(const char* _filename, GLenum _stage)
{
  std::string dir = PIANO_RESOURCE_DIR;
  dir.append("shaders/");
  dir.append(_filename);

  std::vector<char> code = Piano::LoadFile(dir.c_str());
  if (code.size() == 0)
  {
    printf("Failed to open the shader file %s\n", dir.c_str());
    return -1;
  }

  char* c = code.data();
  i32 size = (i32)code.size();

  u32 shader = glCreateShader(_stage);
  glShaderSource(shader, 1, reinterpret_cast<GLchar**>(&c), (GLint*)&size);
  glCompileShader(shader);

  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    printf("Shader (%s) failed to compile\n%s\n", dir.c_str(), infoLog);
  }

  return shader;
}

b8 CreateMaterial(const char* _vert, const char* _frag, Piano::Material* _material, b8 _dynamic)
{
  u32 vs, fs;

  // Create shader stages =====
  vs = LoadShader(_vert, GL_VERTEX_SHADER);
  fs = LoadShader(_frag, GL_FRAGMENT_SHADER);

  _material->vertFile = _vert;
  _material->fragFile = _frag;

  // Setup shader program =====
  _material->shaderProgram = glCreateProgram();
  glAttachShader(_material->shaderProgram, vs);
  glAttachShader(_material->shaderProgram, fs);
  glLinkProgram(_material->shaderProgram);

  int success;
  char infoLog[512];
  glGetProgramiv(_material->shaderProgram, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(_material->shaderProgram, 512, NULL, infoLog);
    PianoLogFatal("Program linking failed\n%s\n", infoLog);
    return false;
  }

  glDeleteShader(vs);
  glDeleteShader(fs);

  // Bind vertex info =====
  glGenBuffers(1, &_material->vbo);
  glGenBuffers(1, &_material->ebo);
  glGenVertexArrays(1, &_material->vao);

  glBindVertexArray(_material->vao);

  const vec3 vertices[4] = {
    { 1.0f, 1.0f, 0.0f}, // TR
    { 1.0f, 0.0f, 0.0f}, // BR
    { 0.0f, 0.0f, 0.0f}, // BL
    { 0.0f, 1.0f, 0.0f}  // TL
  };

  const u32 indices[6] = {
      0, 1, 3,
      1, 2, 3
  };

  glBindBuffer(GL_ARRAY_BUFFER, _material->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _material->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  return true;
}

b8 InitializeFontTextures(FT_Face& _vectorFace)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

  for (u32 c = 0; c < 128; c++)
  {
      // load character glyph 
      if (FT_Load_Char(_vectorFace, c, FT_LOAD_RENDER))
      {
          PianoLogWarning("Failed to load font character '%c'", (char)c);

          if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
            return false;

          continue;
      }
      // generate texture
      unsigned int texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   GL_RED,
                   _vectorFace->glyph->bitmap.width,
                   _vectorFace->glyph->bitmap.rows,
                   0,
                   GL_RED,
                   GL_UNSIGNED_BYTE,
                   _vectorFace->glyph->bitmap.buffer
      );
      // set texture options
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // now store character for later use
      Piano::TextGlyph character =
      {
          texture,
          glm::ivec2(_vectorFace->glyph->bitmap.width, _vectorFace->glyph->bitmap.rows),
          glm::ivec2(_vectorFace->glyph->bitmap_left, _vectorFace->glyph->bitmap_top),
          _vectorFace->glyph->advance.x
      };
      fontGlyphs.insert(std::pair<char, Piano::TextGlyph>(c, character));
  }

  return true;
}

b8 Piano::Renderer::Initialize(Piano::Renderer::RendererSettings _settings)
{
  Piano::Renderer::SetRenderOctaveNotes(true);
  
  windowExtents = _settings.windowExtents;

  // Initialize GLAD / OpenGL =====
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    PianoLogFatal("Failed to initialize GLAD %d", 1);
    return false;
  }

  glViewport(0, 0, _settings.windowExtents.x, _settings.windowExtents.y);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // Initialize Freetype & font =====
  FT_Library freetype;
  FT_Face fontVectorFace;

  if (FT_Init_FreeType(&freetype))
  {
    PianoLogFatal("Failed to initialize FreeType %d", 1);
    return false;
  }

  std::string fontDirectory = PIANO_RESOURCE_DIR;
  //fontDirectory.append("fonts/MADETOMMYRegular_PERSONALUSE.otf");
  fontDirectory.append("fonts/arial.ttf");
  if (FT_New_Face(freetype, fontDirectory.c_str(), 0, &fontVectorFace))
  {
    PianoLogFatal("Failed to create font face %d", 1);
  }

  FT_Set_Pixel_Sizes(fontVectorFace, 0, 48);
  InitializeFontTextures(fontVectorFace);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  screenspaceMatrix = glm::ortho(0.0f, (float)_settings.windowExtents.x, 0.0f, (float)_settings.windowExtents.y);
  //screenspaceMatrix = glm::ortho(0.0f, 100.0f, 0.0f, 100.0f);

  // Create materials =====
  if (!CreateMaterial("note.vert", "note.frag", &noteMaterial, false))
  {
    PianoLogFatal("Failed to create note material %d", 1);
    return false;
  }

  if (!CreateMaterial("text.vert", "text.frag", &textMaterial, false))
  {
    PianoLogFatal("Failed to create text material %d", 1);
    return false;
  }

  // Initialize camera =====
  ChangeCameraSettings(_settings.keyboardViewWidth, _settings.previewDuration);
  PlaceCamera(0.0f);

  return true;
}

b8 Piano::Renderer::Shutdown()
{
  glDeleteVertexArrays(1, &noteMaterial.vao);
  glDeleteBuffers(1, &noteMaterial.vbo);
  glDeleteBuffers(1, &noteMaterial.ebo);
  glDeleteShader(noteMaterial.shaderProgram);

  glDeleteVertexArrays(1, &textMaterial.vao);
  glDeleteBuffers(1, &textMaterial.vbo);
  glDeleteBuffers(1, &textMaterial.ebo);
  glDeleteShader(textMaterial.shaderProgram);

  return true;
}

//=========================
// Update
//=========================

b8 Piano::Renderer::RenderFrame()
{
  glClear(GL_COLOR_BUFFER_BIT);

  // Render octave lines =====
  OctaveLineRender();

  // Render notes =====
  {
    glUseProgram(noteMaterial.shaderProgram);
    u32 noteTransformVectorID = glGetUniformLocation(noteMaterial.shaderProgram, "transformValues");
    u32 viewID = glGetUniformLocation(noteMaterial.shaderProgram, "viewMatrix");
    u32 colorID = glGetUniformLocation(noteMaterial.shaderProgram, "inColor");

    glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));

    glUniform3f(colorID, whiteColor.r, whiteColor.g, whiteColor.b);
    for (const auto& noteValue : whiteKeyTransforms)
    {
      glUniform4fv(noteTransformVectorID, 1, (float*)&noteValue);

      // Render one note quad =====
      glBindVertexArray(noteMaterial.vao);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }

    glUniform3f(colorID, blackColor.r, blackColor.g, blackColor.b);
    for (const auto& noteValue : blackKeyTransforms)
    {
      glUniform4fv(noteTransformVectorID, 1, (float*)&noteValue);

      // Render one note quad =====
      glBindVertexArray(noteMaterial.vao);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }
  }

  static float m = 0.0f;

  std::string text = "A";
  float scale = 2.0f;
  float x = 0.0f;
  float y = 0.0f;
  vec3 color = {1.0f, 0.2f, 1.0f};

  vec4 textTransform = { 0.0f, 0.0f, 1.0f, 1.0f};

  // Render text =====
  {
    glUseProgram(textMaterial.shaderProgram);
    u32 viewID = glGetUniformLocation(textMaterial.shaderProgram, "viewMatrix");
    u32 colorID = glGetUniformLocation(textMaterial.shaderProgram, "inTextColor");
    u32 transformID = glGetUniformLocation(textMaterial.shaderProgram, "transformValues");
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textMaterial.vao);

    glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
    for (const auto& text : worldspaceText)
    {
      glUniform3f(colorID, text.color.x, text.color.y, text.color.z);
      glUniform4f(transformID,
                  text.transform.x,
                  text.transform.y,
                  text.transform.z,
                  text.transform.w);

      glBindTexture(GL_TEXTURE_2D, text.textureID);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(screenspaceMatrix));
    for (const auto& text : screenspaceText)
    {
      glUniform3f(colorID, text.color.x, text.color.y, text.color.z);
      glUniform4f(transformID,
                  text.transform.x,
                  text.transform.y,
                  text.transform.z,
                  text.transform.w);

      glBindTexture(GL_TEXTURE_2D, text.textureID);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
  }

  return true;
}

//=========================
// Functionality
//=========================

void Piano::Renderer::ChangeCameraSettings(float _width, f32 _previewDuration)
{
  projectionMatrix = glm::ortho(0.0f, _width, 0.0f, _previewDuration);
}

void Piano::Renderer::PlaceCamera(f32 _time)
{
  glm::mat4 view = identityMatrix;
  view[3][1] = -_time;
  viewProjectionMatrix = projectionMatrix * view;
}

void Piano::Renderer::SetNotes(const std::vector<Piano::note>& _notes)
{
  whiteKeyTransforms.clear();
  whiteKeyTransforms.reserve(_notes.size());
  blackKeyTransforms.clear();
  blackKeyTransforms.reserve(_notes.size());
  for (auto& n : _notes)
  {
    if (n.keyWidth == BLACK_KEY_WIDTH)
    {
      blackKeyTransforms.push_back(n);
    }
    else
    {
      whiteKeyTransforms.push_back(n);
    }
  }
}

void Piano::Renderer::AddText(const std::string& _text,
                              vec2 _startPosition,
                              vec3 _color /*= {1.0f, 1.0f, 1.0f}*/,
                              f32 _scale /*= 1.0f*/,
                              b8 _printToWorldspace /*= false*/)
{

  std::vector<textRenderData>& renderedText = _printToWorldspace ? worldspaceText : screenspaceText;
  float xScale = _printToWorldspace ? _scale / ((float)windowExtents.x * projectionMatrix[0][0]) : _scale;
  float yScale = _printToWorldspace ? _scale / ((float)windowExtents.y * projectionMatrix[1][1]) : _scale;

  renderedText.reserve(renderedText.size() + _text.size());

  f32 xPosition, yPosition, width, height;
  Piano::TextGlyph glyph;

  textRenderData newChar {};

  f32 originX = _startPosition.x;
  f32 originY = _startPosition.y;

  for (const auto& c : _text)
  {
    glyph = fontGlyphs[c];

    xPosition = originX + (glyph.bearing.x * xScale);
    yPosition = originY - ((glyph.size.y - glyph.bearing.y) * yScale);
    width = glyph.size.x * xScale;
    height = glyph.size.y * yScale;

    newChar.transform = {xPosition, yPosition, width, height};
    newChar.textureID = glyph.textureID;
    newChar.color = _color;

    renderedText.push_back(newChar);

    f32 pre = originX;
    originX += (float)(glyph.advance >> 6) * xScale;
  }
}

void Piano::Renderer::ClearText(b8 _clearWorldText)
{
  if (_clearWorldText)
  {
    worldspaceText.clear();
  }
  else
  {
    screenspaceText.clear();
  }
}

void Piano::Renderer::SetRenderOctaveNotes(b8 _value)
{
  OctaveLineRender = _value ? RenderOctaveLines : DoNothing;
}

void RenderOctaveLines()
{
  glUseProgram(noteMaterial.shaderProgram);
  u32 noteTransformVectorID = glGetUniformLocation(noteMaterial.shaderProgram, "transformValues");
  u32 viewID = glGetUniformLocation(noteMaterial.shaderProgram, "viewMatrix");
  u32 colorID = glGetUniformLocation(noteMaterial.shaderProgram, "inColor");

  glUniformMatrix4fv(viewID, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

  glUniform3f(colorID, octaveColor.r, octaveColor.g, octaveColor.b);
  Piano::note tmpNote = {0, -1.0f, octaveWidth, -5.0f};
  
  for (u32 i = 0; i < 10; i++)
  {
    tmpNote.keyPosition = i * 7.0f - ((octaveWidth * 0.5f + WHITE_KEY_WIDTH * 0.2f) * (i > 0));
    glUniform4fv(noteTransformVectorID, 1, (float*)&tmpNote);

    // Render one note quad =====
    glBindVertexArray(noteMaterial.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  }
}
