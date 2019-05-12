#include <stdio.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4dp.h>
#include <GL4D/gl4dh.h>
#include <GL4D/gl4duw_SDL2.h>
#include <SDL_mixer.h>
#include <assert.h>
#include "audioHelper.h"

typedef struct mobile_t mobile_t;
struct mobile_t {
  double x, y;
  GLuint c;
  float r;
};

typedef struct line_t line_t;
struct line_t {
  double x0, y0, x1, y1;
  GLuint c;
};

static void init(void);
static void draw(void);
static void quit(void);


static mobile_t * _mobile;
static line_t * _line;
static int _nb_mobiles = 0;
static int _nb_lines = 0;
static int _lineSimu = 0;
static GLuint _screen = 0;
static GLuint _red = RGB(217, 80, 64);
static GLuint _yellow = RGB(249, 189, 66);
static GLuint _green = RGB(88, 164, 66);
static GLuint _blue = RGB(78, 136, 237);

static void init(void) {
  int i;
  _nb_mobiles = 4;
  _nb_lines = _nb_mobiles;
  double pos_mobile[4][2] = {
    {gl4dpGetWidth()/2 - gl4dpGetWidth()/4, gl4dpGetHeight()/2 - gl4dpGetHeight()/4},
    {gl4dpGetWidth()/2 + gl4dpGetWidth()/4, gl4dpGetHeight()/2 - gl4dpGetHeight()/4},
    {gl4dpGetWidth()/2 + gl4dpGetWidth()/4, gl4dpGetHeight()/2 + gl4dpGetHeight()/4},
    {gl4dpGetWidth()/2 - gl4dpGetWidth()/4, gl4dpGetHeight()/2 + gl4dpGetHeight()/4}
  };

  _mobile = malloc(_nb_mobiles * sizeof(*_mobile));
  assert(_mobile);

  _line = malloc(_nb_mobiles * sizeof(*_line));
  assert(_line);

  for(i = 0; i < _nb_mobiles; i++) {
    _line[i].x0 = _line[i].x1 = _mobile[i].x = pos_mobile[i][0];
    _line[i].y0 = _line[i].y1 = _mobile[i].y = pos_mobile[i][1];
    _mobile[i].r = 0;

    switch(i) {
      case 0:
        _mobile[i].c = _red;
        break;
      case 1:
        _mobile[i].c = _yellow;
        break;
      case 2:
        _mobile[i].c = _green;
        break;
      default:
        _mobile[i].c = _blue;
        break;
    }
  }
}

static void mobileSimu(void) {
  if(_lineSimu)
    return;

  static int idx = 0;
  _mobile[idx].r += 0.1; 
  if(_mobile[idx].r >= 10.0) {
    if(idx >= _nb_mobiles-1) {
      _lineSimu = 1;
      return;
    }
    idx++;
  }
}

static void lineSimu(void) {
  static int i = 0, end = 0;
  if(end || !_lineSimu)
    return;

  float x, y, d;
  int sec;
  d = 3.0 ;
  switch(i) {
    case 0:
      x = d; y = 0;
      break;
    case 1:
      x = 0; y = d;
      break;
    case 2:
      x = -d; y = 0;
      break;
    case 3:
      x = 0; y = -d;
      break;
  }

  if(_lineSimu) {
    _line[i].x1 += x;
    _line[i].y1 += y;

    sec = (i + 1) % _nb_mobiles;
    if(_line[i].x1 >= _mobile[sec].x &&
       i == 0) {
      i++;
      return;
    }
    if(_line[i].x1 == _mobile[sec].x &&
       _line[i].y1 >= _mobile[sec].y && 
       i == 1) {
      i++;
      return;
    }

    if(_line[i].x1 <= _mobile[sec].x &&
       _line[i].y1 == _mobile[sec].y && 
       i == 2) {
      i++;
      return;
    }

    if(_line[i].x1 == _mobile[sec].x &&
       _line[i].y1 <= _mobile[sec].y && 
       i == 3) {
      i++;
      end = 1;
      return;
    }
  }
}
static void draw(void) {
  int i;
  gl4dpClearScreenWith(RGB(255, 255, 255));
  mobileSimu();
  lineSimu();
  for(i = 0; i < _nb_mobiles; i++) {
    gl4dpSetColor(_mobile[i].c);
    gl4dpFilledCircle(_mobile[i].x, _mobile[i].y, _mobile[i].r);
    gl4dpLine(_line[i].x0, _line[i].y0, _line[i].x1, _line[i].y1);
  }
}


static void quit(void) {
  if(_screen) {
    gl4dpSetScreen(_screen);
    gl4dpDeleteScreen();
    _screen = 0;
  }
}

void point(int state) {
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
    gl4dpUpdateScreen(NULL);
    return;
  }
}
