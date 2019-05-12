#include <assert.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4duw_SDL2.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <fftw3.h>
#include <GL4D/gl4dh.h>
#include "audioHelper.h"

typedef struct mobile_t mobile_t;
struct mobile_t {
  double x, y, vx, vy;
  float color[3];
  float r;
  int alive;
};

#define MAX_COLOR 255

static void         init(void);
static void         draw(void);
static void         quit(void);
static void         mobileInit(void);
static void         mobile2texture(float * f);
static void         mobileSimu(void);

static GLuint _pId = 0;
static GLuint _tId = 0;
static GLuint _screen = 0;

static mobile_t * _mobile = NULL;
static const int _nb_mobiles = 8;

static GLuint _quad = 0;
static float _largeur = 0;
static int _voronoi = 0;

static void mobileInit(void) {
  int i;
  srand(time(NULL));

  if(_mobile) {
    free(_mobile);
    _mobile = NULL;
  }

  _mobile = malloc(_nb_mobiles * sizeof *_mobile);
  assert(_mobile);

  for(i = 0; i < _nb_mobiles; i++) {
    _mobile[i].r = 0;
    _mobile[i].x = gl4dpGetWidth() / 4 + gl4dmURand() * gl4dpGetWidth() / 2;
    _mobile[i].y =  (i%2 == 0 ? gl4dpGetHeight()/2 + gl4dmURand() * (gl4dpGetHeight() / 2) :
                           gl4dpGetHeight()/2 - gl4dmURand() * (gl4dpGetHeight() / 2));
    _mobile[i].vx = gl4dpGetWidth()  * ((rand() / (RAND_MAX + 1.0)) - 0.5);
    _mobile[i].vy = gl4dpGetHeight() * ((rand() / (RAND_MAX + 1.0)) - 0.5);
    _mobile[i].color[0] = gl4dmURand();
    _mobile[i].color[1] = gl4dmURand();
    _mobile[i].color[2] = gl4dmURand();
  }
}

static void init(void) {
  _pId = gl4duCreateProgram("<vs>shaders/voronoi.vs", "<fs>shaders/voronoi.fs", NULL);
  _quad = gl4dgGenQuadf();
  glGenTextures(1, &_tId);
  glBindTexture(GL_TEXTURE_1D, _tId);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, 1, 0, GL_RGBA, GL_FLOAT, NULL);
  glBindTexture(GL_TEXTURE_1D, 0);
  mobileInit();
}

static void mobile2texture(float * f) {
  int i;
  for(i = 0; i < _nb_mobiles; i++) {
    f[8 * i + 0] = _mobile[i].color[0];
    f[8 * i + 1] = _mobile[i].color[1];
    f[8 * i + 2] = _mobile[i].color[2];
    f[8 * i + 3] = 1;
    f[8 * i + 4] = _mobile[i].x / gl4dpGetWidth();
    f[8 * i + 5] = _mobile[i].y / gl4dpGetHeight();
    f[8 * i + 6] = _mobile[i].r / ((GLfloat)MIN(gl4dpGetWidth(), gl4dpGetHeight()));
    f[8 * i + 7] = 1;
  }
}

static float mobileMaxHeight(void) {
  float max = _mobile[0].y;
  int i;
  for(i = 1; i < _nb_mobiles; i++) {
    if(_mobile[i].y > max) {
      max = _mobile[i].y; 
    }
  }

  return max;
}


static void frottements(int i, float kx, float ky) {
  const float dt = 1.0 / 60.0;
  double x = fabs(_mobile[i].vx), y = fabs(_mobile[i].vy);
  _mobile[i].vx = MAX(x - kx * x * x * dt * dt - 2.0 * kx, 0.0) * SIGN(_mobile[i].vx);
  _mobile[i].vy = MAX(y - ky * y * y * dt * dt - 2.0 * ky, 0.0) * SIGN(_mobile[i].vy);
}


static void mobileSimu(void) {
  static int idx = 0;
  static int start = 0;
  if(_voronoi) {
    _largeur = mobileMaxHeight();
    _largeur = _largeur / gl4dpGetHeight();
    static Uint32 t0 = 0;
    Uint32 t = SDL_GetTicks();
    if(t >= 129000) 
      _voronoi = 2;
    double dt = (t - t0) / 1000.0, d;
    t0 = t;
    int i, collision_x, collision_bas;

    for(i = 0; i < _nb_mobiles; i++) {
      collision_x = collision_bas = 0;
      if(start) {
        _mobile[i].x += _mobile[i].vx * dt;
        _mobile[i].y += _mobile[i].vy * dt;
      }
      if( (d = _mobile[i].x - _mobile[i].r) <= 0 ) {
        if(_mobile[i].vx < 0)
          _mobile[i].vx = -_mobile[i].vx;
        _mobile[i].vx -= d;
        frottements(i, 1.0, 0.2);
        collision_x = 1;
      }
      if( (d = _mobile[i].x + _mobile[i].r - (gl4dpGetWidth() - 1)) >= 0 ) {
        if(_mobile[i].vx > 0)
          _mobile[i].vx = -_mobile[i].vx;
        _mobile[i].vx -= d;
        frottements(i, 1.0, 0.2);
        collision_x = 1;
      }
      if( (d = _mobile[i].y - (_mobile[i].r + 1)) <= 0 ) {
        if(_mobile[i].vy < 0)
          _mobile[i].vy = -_mobile[i].vy;
        _mobile[i].vy -= d;
        frottements(i, 0.2, 1.0);
        collision_bas = 1;
      }

      if( (d = _mobile[i].y + (_mobile[i].r + 1)) - (gl4dpGetHeight() - 1) >= 0 ) {
        if(_mobile[i].vy > 0)
          _mobile[i].vy = -_mobile[i].vy;
        _mobile[i].vy -= d;
        frottements(i, 0.2, 1.0);
        collision_bas = 1;
      }
    }
    start++;
    return;
  }

  if(_mobile[idx].r < 11)
    _mobile[idx].r += 0.1;
  else
    idx++;
  if(idx >= _nb_mobiles)
    _voronoi = 1;
}

static void draw(void) {
  GLfloat * f = malloc(_nb_mobiles * 8 * sizeof *f), step = 1.0 / (_nb_mobiles * 2);
  assert(f);
  glDisable(GL_DEPTH_TEST);
  glUseProgram(_pId);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);  

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_1D, _tId);
  mobileSimu();
  mobile2texture(f);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, 2 * _nb_mobiles, 0, GL_RGBA, GL_FLOAT, f);
  free(f);
  glUniform1i(glGetUniformLocation(_pId, "voronoi"), _voronoi);
  glUniform1f(glGetUniformLocation(_pId, "largeur"), _largeur);
  glUniform1i(glGetUniformLocation(_pId, "mobiles"), 0);
  glUniform1f(glGetUniformLocation(_pId, "step"), step);

  gl4dgDraw(_quad);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_1D, 0);
  glUseProgram(0);
}

static void quit(void) {
  if(_mobile) {
    free(_mobile);
    _mobile = NULL;
  }

  if(_tId) {
    glDeleteTextures(1, &_tId);
    _tId = 0;
  }

  if(_screen) {
    gl4dpSetScreen(_screen);
    gl4dpDeleteScreen();
    _screen = 0;
  }
}


void voronoi(int state) {
  switch(state) {
  case GL4DH_INIT:
    _screen = gl4dpInitScreen();
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
