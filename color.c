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

static void init(void);
static void draw(void);
static void quit(void);


static mobile_t * _mobile;
static int _nb_mobiles = 0;
static GLuint _screen = 0;
static GLuint _red = RGB(217, 80, 64);
static GLuint _yellow = RGB(249, 189, 66);
static GLuint _green = RGB(88, 164, 66);
static GLuint _blue = RGB(78, 136, 237);
static int _onde = 0;
static char _pi[] = "3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214808651328230664709384460955058223172535940812848111745028410270193852110555964462294895493038196442881097566593344612847564823378678316527120190914564856692346034861045432664821339360726024914127372458700660631558817488152092096282925409171536436789259036001133053054882046652138414695194151160943305727036575959195309218611738193261179310511854807446237996274956735188575272489122793818301194912983367336244065664308602139494639522473719070217986094370277053921717629317675238467481846766940513200056812714526356082778577134275778960917363717872146844090122495343014";
static int _idx = 0;

static void init(void) {
  int i;
  _nb_mobiles = 4;

  _mobile = malloc(_nb_mobiles * sizeof(*_mobile));
  assert(_mobile);

  for(i = 0; i < _nb_mobiles; i++) {
    _mobile[i].x = gl4dpGetWidth()/2;
    _mobile[i].y = gl4dpGetHeight()/2;
    _mobile[i].r = 0;

    switch(i) {
      case 0: case 4:
        _mobile[i].c = _red;
        break;
      case 1: case 5:
        _mobile[i].c = _yellow;
        break;
      case 2: case 6:
        _mobile[i].c = _green;
        break;
      default:
        _mobile[i].c = _blue;
        break;
    }
  }
}

void mobileSimu(void) {
  static int i = 0;
  if(!_onde) {
    if(_mobile[i].x + _mobile[i].r <= gl4dpGetWidth() &&
       _mobile[i].y + _mobile[i].r <= gl4dpGetHeight())
      _mobile[i].r += 1.5;
    else {
      i = (i + 1) % _nb_mobiles;
      if(i == 0)
        _onde = 1;
    }
  }
}

static void draw(void) {
  static int r = 0;
  int i, sgn = 1, t;
  mobileSimu();
  if(!_onde && !r) {
    gl4dpClearScreenWith(RGB(255, 255, 255));
    for(i = 0; i < _nb_mobiles; i++) {
      gl4dpSetColor(_mobile[i].c);
      gl4dpFilledCircle(_mobile[i].x, _mobile[i].y, _mobile[i].r);
    }
    if(_onde) r = 1;
  }

  if(_onde) {
    int dig = _pi[_idx] - '0';
    int nxtDig = _pi[_idx+1] - '0';
    _idx++;

    float diff = (2 * M_PI) / 4;
    float a1 = dig * 4 / (2 * M_PI) + gl4dmURand() * diff * sgn;
    float a2 = nxtDig * 4 / (2 * M_PI) + gl4dmURand() * diff * sgn;

    float x1 = _mobile[_nb_mobiles-1].r * cos(a1) + gl4dpGetWidth()/2;
    float y1 = _mobile[_nb_mobiles-1].r * sin(a1) + gl4dpGetHeight()/2;

    float x2 = _mobile[_nb_mobiles-1].r * cos(a2) + gl4dpGetWidth()/2;
    float y2 = _mobile[_nb_mobiles-1].r * sin(a2) + gl4dpGetHeight()/2;

    gl4dpSetColor(RGB(255, 255, 255));
    for(t = 0; t < 2; t++) {
      if(x2 - t >= 0 && y2 - t >= 0 && x2 - t >= 0 && y2 - t >= 0) 
        gl4dpLine(x1 - t, y1 - t, x2 - t, y2 - t);
      gl4dpLine(x1, y1, x2, y2);
      if(x2 + t <= gl4dpGetWidth()-1 && y2 + t <= gl4dpGetHeight()-1 && 
         x1 + t <= gl4dpGetWidth()-1 && y1 + t <= gl4dpGetHeight()-1) 
        gl4dpLine(x1 + t, y1 + t, x2 + t, y2 + t);
    }

    sgn = (sgn > 0 ? -1 : 1);
  }
}


static void quit(void) {
  if(_screen) {
    gl4dpSetScreen(_screen);
    gl4dpDeleteScreen();
    _screen = 0;
  }
}

void coloration(int state) {
  switch(state) {
  case GL4DH_INIT:
    _screen = gl4dpInitScreen();
    init();
    return;
  case GL4DH_FREE:
    quit();
    gl4dpSetScreen(_screen);
    gl4dpDeleteScreen();
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    return;
  default:
    draw();
    gl4dpUpdateScreen(NULL);
    return;
  }
}
