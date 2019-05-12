/*!\file window.c
 *
 * \brief Lumière positionnelle + Phong + Bump mapping + Normal
 * mapping + textures et geometry shader
 *
 */
#include <assert.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4duw_SDL2.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <fftw3.h>
#include <GL4D/gl4dh.h>
#include "audioHelper.h"

/* Prototypes des fonctions statiques contenues dans ce fichier C */
static void         init(void);
static void         draw(void);
static void         quit(void);

/*!\brief identifiant des GLSL program */
static GLuint _pId = 0;
static GLuint _texId = 0;
/*!\brief identifiant de la sphere */
static GLuint _vao = 0;
static GLuint _buffer[2] = { 0 };
static float _red[3] = { 0.85f, 0.31f, 0.25f };
static float _yellow[3] = { 0.97f, 0.74f, 0.25f };
static float _green[3] = { 0.34f, 0.64f, 0.25f };
static float _blue[3] = { 0.30f, 0.53f, 0.92f };
static float _screen_color[3] = { 0.0, 0.0, 0.0 };

static void init(void) {
  /* indices pour réaliser le maillage des géométrie, envoyés dans le
   * VBO ELEMENT_ARRAY_BUFFER */
  GLuint idata[] = {
    /* un quadrilatère en triangle strip (d'où inversion entre 3 et 2) */
    0, 1, 3, 2
  };
  /* données-sommets envoyée dans le VBO ARRAY_BUFFER */
  GLfloat data[] = {
    /* un sommet est composé d'une coordonnée 3D, d'une couleur 3D et
     * d'une coordonnée de texture 2D */
    /* sommet  0 */ -1, -1, 0, _red[0], _red[1], _red[2],  50,  50,
    /* sommet  1 */  1, -1, 0, _yellow[0], _yellow[1], _yellow[2], 50,  50,
    /* sommet  2 */  1,  1, 0, _green[0], _green[1], _green[2], 50, 50,
    /* sommet  3 */ -1,  1, 0, _blue[0], _blue[1], _blue[2],  50, 50
  };

  const GLuint B = RGB(255, 255, 255);
  GLuint tex[] = { B, B, B, B };
  /* Génération d'un identifiant de VAO */
  glGenVertexArrays(1, &_vao);
  /* Lier le VAO-machine-GL à l'identifiant VAO généré */
  glBindVertexArray(_vao);
  /* Activation des 3 premiers indices d'attribut de sommet */
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  /* Génération de deux identifiants de VBO un pour ARRAY_BUFFER, un
   * pour ELEMENT_ARRAY_BUFFER */
  glGenBuffers(2, _buffer);
  /* Lier le VBO-ARRAY_BUFFER à l'identifiant du premier VBO généré */
  glBindBuffer(GL_ARRAY_BUFFER, _buffer[0]);
  /* Transfert des données VBO-ARRAY_BUFFER */
  glBufferData(GL_ARRAY_BUFFER, sizeof data, data, GL_STATIC_DRAW);
  /* Paramétrage 3 premiers indices d'attribut de sommet */
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof *data, (const void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof *data, (const void *)(3 * sizeof *data));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof *data, (const void *)(6 * sizeof *data));
  /* Lier le VBO-ELEMENT_ARRAY_BUFFER à l'identifiant du second VBO généré */
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffer[1]);
  /* Transfert des données d'indices VBO-ELEMENT_ARRAY_BUFFER */
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idata, idata, GL_STATIC_DRAW);
  /* dé-lier le VAO puis les VAO */
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  /* générer un identifiant de texture */
  glGenTextures(1, &_texId);
  /* lier l'identifiant de texture comme texture 2D (1D ou 3D
   * possibles) */
  glBindTexture(GL_TEXTURE_2D, _texId);
  /* paramétrer la texture, voir la doc de la fonction glTexParameter
   * sur
   * https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexParameter.xhtml */
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  /* envoi de la donnée texture depuis la RAM CPU vers la RAM GPU voir
   * la doc de glTexImage2D (voir aussi glTexImage1D et glTexImage3D)
   * sur
   * https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
  /* dé-lier la texture 2D */
  glBindTexture(GL_TEXTURE_2D, 0);

  /* Création du programme shader (voir le dossier shader) */
  _pId = gl4duCreateProgram("<vs>shaders/quad.vs", "<fs>shaders/quad.fs", NULL);
  /* Set de la couleur (RGBA) d'effacement OpenGL */
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  /* activation de la texture 2D */
  /* activation du test de profondeur afin de prendre en compte la
   * notion de devant-derrière. */
  /* Création des matrices GL4Dummies, une pour la projection, une
   * pour la modélisation et une pour la vue */
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");
  gl4duGenMatrix(GL_FLOAT, "modelMatrix");
  gl4duGenMatrix(GL_FLOAT, "viewMatrix");
  /* on active la matrice de projection créée précédemment */
  gl4duBindMatrix("projectionMatrix");
  /* la matrice en cours reçoit identité (matrice avec des 1 sur la
   * diagonale et que des 0 sur le reste) */
  gl4duLoadIdentityf();
  /* on multiplie la matrice en cours par une matrice de projection
   * orthographique ou perspective */
  /* décommenter pour orthographique gl4duOrthof(-1, 1, -1, 1, 0, 100); */
}

static void draw(void) {
  static float scx = 0.7, scy = 0.6; 
  static int c, t = 0;
  GLint vp[4];

  GLboolean gdt = glIsEnabled(GL_DEPTH_TEST);
  glGetIntegerv(GL_VIEWPORT, vp);

  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  gl4duFrustumf(-0.5, 0.5, -0.5 * vp[3] / vp[2], 0.5 * vp[3] / vp[2], 1.0, 1000.0);
  gl4duBindMatrix("modelViewMatrix");
  glClearColor(_screen_color[0], _screen_color[1], _screen_color[2], 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(_pId);
  gl4duBindMatrix("viewMatrix");
  gl4duLoadIdentityf();
    gl4duLookAtf(0, 0, 3, 0, 0, 0, 0, 1, 0);
  gl4duBindMatrix("modelMatrix");

  gl4duLoadIdentityf();
  gl4duScalef(cos(scx), sin(scy), scy);
  gl4duRotatef(1, 0, 1, 0);
  scx += 0.008;
  scy -= 0.008;

  printf("t %d\n", t);
  if(fabs(cos(scx)) >= .99) {
    t++;
    c = gl4dmURand() * 4;
    switch(c) {
      case 0:
        _screen_color[0] = _red[0];
        _screen_color[1] = _red[1];
        _screen_color[2] = _red[2];
        return;
      case 1:
        _screen_color[0] = _yellow[0];
        _screen_color[1] = _yellow[1];
        _screen_color[2] = _yellow[2];
        return;
      case 2:
        _screen_color[0] = _green[0];
        _screen_color[1] = _green[1];
        _screen_color[2] = _green[2];
        return;
      case 3:
        _screen_color[0] = _blue[0];
        _screen_color[1] = _blue[1];
        _screen_color[2] = _blue[2];
        return;
    }
  }


  if(t < 70 && t > 0) {
    _screen_color[0] = _red[0];
    _screen_color[1] = _red[1];
    _screen_color[2] = _red[2];
  }

  else if(t < 105 && t >= 70) {
    _screen_color[0] = _yellow[0];
    _screen_color[1] = _yellow[1];
    _screen_color[2] = _yellow[2];
  }

  else if(t < 141 && t >= 105) {
    _screen_color[0] = _green[0];
    _screen_color[1] = _green[1];
    _screen_color[2] = _green[2];
  }

  else if(t >= 141) {
    _screen_color[0] = _blue[0];
    _screen_color[1] = _blue[1];
    _screen_color[2] = _blue[2];
  }


  gl4duSendMatrices();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _texId);
  glUniform1i(glGetUniformLocation(_pId, "tex"), 0);
  glUniform1i(glGetUniformLocation(_pId, "inv"), 1);   
  glBindVertexArray(_vao);

  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (const GLvoid *)0);
  glBindVertexArray(0);
  glUseProgram(0);

  if(!gdt)
    glDisable(GL_DEPTH_TEST);
}

static void quit(void) {

  if(_texId)
    glDeleteTextures(1, &_texId);

  if(_vao)
    glDeleteVertexArrays(1, &_vao);

  if(_buffer[0])
    glDeleteBuffers(2, _buffer);
}

void rectangle(int state) {
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
