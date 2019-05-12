#include <stdio.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4df.h>
#include <GL4D/gl4duw_SDL2.h>
#include <SDL_image.h>
#include <GL4D/gl4dh.h>
#include "audioHelper.h"

static void init(void);
static void loadTexture(GLuint id, const char * filename);
static void draw(void);
static void quit(void);

static GLuint _pId = 0;
static GLuint _sphere = 0, _cube = 0, _quad = 0, _torus = 0, _tex[5] = {0};
static float _alpha = 1.00001f;

static void init(void) {
  GLint vp[4];
  glGetIntegerv(GL_VIEWPORT, vp);

  _pId  = gl4duCreateProgram("<vs>shaders/depTex.vs", "<fs>shaders/depTex.fs", NULL);
  gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  _sphere = gl4dgGenSpheref(30, 30);
  _cube = gl4dgGenCubef();
  _quad = gl4dgGenQuadf();
  _torus = gl4dgGenTorusf(300, 30, 0.1f);

  glGenTextures(sizeof _tex / sizeof *_tex, _tex);

  loadTexture(_tex[0], "images/sphere.jpg");
  loadTexture(_tex[1], "images/rainbow.jpg");
  loadTexture(_tex[2], "images/quad.jpg");
  loadTexture(_tex[3], "images/torus.jpg");

  glBindTexture(GL_TEXTURE_2D, _tex[4]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vp[2] / 4, vp[3] / 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glEnable(GL_TEXTURE_2D);
}

static void loadTexture(GLuint id, const char * filename) {
  SDL_Surface * t;
  glBindTexture(GL_TEXTURE_2D, id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  if( (t = IMG_Load(filename)) != NULL ) {
#ifdef __APPLE__
    int mode = t->format->BytesPerPixel == 4 ? GL_BGRA : GL_BGR;
#else
    int mode = t->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
#endif       
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, mode, GL_UNSIGNED_BYTE, t->pixels);
    SDL_FreeSurface(t);
  } else {
    fprintf(stderr, "can't open file %s : %s\n", filename, SDL_GetError());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  }
}

static void draw(void) {
  static GLfloat a = 0, coef = 0;
  static float tc = -500;
  int t, totalTemps = 34000;
  float blue_r = 0.30f;
  float blue_g = 0.53f;
  float blue_b = 0.92f;

  t = SDL_GetTicks() - 39650;

  _alpha = (float)(((totalTemps - (totalTemps - t)) / (float)totalTemps));
  GLint vp[4];
  GLboolean dt = glIsEnabled(GL_DEPTH_TEST);
  glGetIntegerv(GL_VIEWPORT, vp);
  glEnable(GL_DEPTH_TEST);

  if(blue_r - _alpha >= 0)
    blue_r -= _alpha;
  else
    blue_r = 0;

  if(blue_g - _alpha >= 0)
    blue_g -= _alpha;
  else
    blue_g = 0;

  if(blue_b - _alpha >= 0)
    blue_b -= _alpha;
  else
    blue_b = 0;

  glClearColor(blue_r, blue_g, blue_b, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  gl4duFrustumf(-0.5, 0.5, -0.5 * vp[3] / vp[2], 0.5 * vp[3] / vp[2], 1.0, 1000.0);
  gl4duBindMatrix("modelViewMatrix");

  gl4duLoadIdentityf();
  glUseProgram(_pId);
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(_pId, "tex"), 0);
  glUniform1f(glGetUniformLocation(_pId, "couleur"), coef);
  gl4duTranslatef(0, 0, -50.0);

  gl4duTranslatef(0, 0, tc);
  gl4duRotatef(-a, sin(a), 0, 0);
  gl4duSendMatrices();
  glBindTexture(GL_TEXTURE_2D, _tex[1]);
  gl4dgDraw(_cube);
  if(tc < 40.0)
    tc += 1.5;

  gl4duRotatef(a, 0, 1, 0);
  gl4duTranslatef(3, 0, 0);
  gl4duPushMatrix(); {
    gl4duScalef(0.4f, 0.4f, 0.4f);
    gl4duSendMatrices();
  } gl4duPopMatrix();
  glBindTexture(GL_TEXTURE_2D, _tex[1]);
  gl4dgDraw(_sphere);
  gl4duRotatef(-3 * a, 1, 0, 0);
  gl4duSendMatrices();
  glBindTexture(GL_TEXTURE_2D, _tex[3]);
  gl4dgDraw(_torus);

  coef += 0.01;
  a += 0.5 * coef;
  glUseProgram(0);
  if(!dt)
    glDisable(GL_DEPTH_TEST);
}

static void quit(void) {
  glDeleteTextures(sizeof _tex / sizeof *_tex, _tex);
}


void rainbow(int state) {
  switch(state) {
  case GL4DH_INIT:
    init();
    return;
  case GL4DH_FREE:
    quit();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    return;
  default:
    draw();
    return;
  }
}
