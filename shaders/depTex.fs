/*!\file depTex.fs
 *
 * \brief rendu avec lumière directionnelle diffuse et texture.
 * \author Farès BELHADJ, amsi@ai.univ-paris8.fr 
 * \date May 13 2018
 */
#version 330
uniform sampler2D tex;
uniform float temps;
uniform float couleur;
in  vec2 vsoTexCoord;
in  vec3 vsoNormal;
out vec4 fragColor;

void main(void) {
  vec2 vecteur = vsoTexCoord - vec2(0.5);
  float distance = length(vecteur);
  float angle = atan(vecteur.y, vecteur.x);
  angle +=  0.01 * temps / (1.0 + distance);
  vec2 tc = vec2(0.5) + vec2(distance * cos(angle), distance * sin(angle));

  vec3 N = normalize(vsoNormal);
  vec3 L = normalize(vec3(-1, -1, 0));
  float diffuse = dot(N, -L);
  vec4 t = mix(vec4((texture(tex, vsoTexCoord).rgb), 1), vec4(tc, 1.0, 1.0), smoothstep(0.5, 0.1, cos(couleur)));
  fragColor = t;
}
