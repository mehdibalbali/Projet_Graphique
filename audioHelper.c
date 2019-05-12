
#include "audioHelper.h"

/*!\brief pointeur vers la musique chargée par SDL_Mixer */
static Mix_Music * _mmusic = NULL;

/*!\brief pointeur vers le flux audio.
 * \see ahGetAudioStream
 * \see ahSetAudioStream
 */
static Uint8 * _audioStream = NULL;
/*!\brief longueur du flux audio.
 * \see ahGetAudioStreamLength
 * \see ahSetAudioStream
 */
static int _audioStreamLength = 0;

/*!\brief renvoie le pointeur vers le flux audio.
 * \return le pointeur vers le flux audio.
 */
Uint8 * ahGetAudioStream(void) {
  return _audioStream;
}

/*!\brief renvoie la longueur du flux audio.
 * \return la longueur du flux audio.
 */
int ahGetAudioStreamLength(void) {
  return _audioStreamLength;
}

/*!\brief met à jour le pointeur vers le flux audio et sa longueur.
 */
void ahSetAudioStream(Uint8 * audioStream, int audioStreamLength) {
  _audioStream = audioStream;
  _audioStreamLength = audioStreamLength;
}

/*!\brief Cette fonction est appelée quand l'audio est joué et met 
 * dans \a stream les données audio de longueur \a len.
 * \param udata pour user data, données passées par l'utilisateur, ici NULL.
 * \param stream flux de données audio.
 * \param len longueur de \a stream.
 */
static void mixCallback(void *udata, Uint8 *stream, int len) {
  ahSetAudioStream(stream, len);
  gl4dhUpdateWithAudio();
  ahSetAudioStream(NULL, 0);
}

/*!\brief Cette fonction initialise les paramètres SDL_Mixer et charge
 *  le fichier audio.
 */
void ahInitAudio(const char * file) {
#if defined(__APPLE__)
  int mult = 1;
#else
  int mult = 2;
#endif
  int mixFlags = MIX_INIT_MP3, res;
  res = Mix_Init(mixFlags);
  if( (res & mixFlags) != mixFlags ) {
    fprintf(stderr, "Mix_Init: Erreur lors de l'initialisation de la bibliotheque SDL_Mixer\n");
    fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
    //exit(3); commenté car ne réagit correctement sur toutes les architectures
  }
  if(Mix_OpenAudio(44100, AUDIO_S16LSB, 1, mult * 1024) < 0)
    exit(4);  
  if(!(_mmusic = Mix_LoadMUS(file))) {
    fprintf(stderr, "Erreur lors du Mix_LoadMUS: %s\n", Mix_GetError());
    exit(5);
  }
  Mix_SetPostMix(mixCallback, NULL);
  if(!Mix_PlayingMusic())
    Mix_PlayMusic(_mmusic, 1);
}

/*!\brief Libère l'audio.
 */
void ahClean(void) {
  if(_mmusic) {
    if(Mix_PlayingMusic())
      Mix_HaltMusic();
    Mix_FreeMusic(_mmusic);
    _mmusic = NULL;
  }
  Mix_CloseAudio();
  Mix_Quit();
}
#include <GL4D/gl4du.h>