#version 330
uniform sampler1D mobiles;
uniform float step;
uniform int voronoi;
uniform float largeur;
in  vec2 vsoTexCoord;
out vec4 fragColor;

vec4 balle(void) {
  float i;
  for(i = step / 2.0; i < 1.0; i += 2.0 * step) {
    vec3 po = texture(mobiles, i + step).xyz;
    float d = length(vsoTexCoord - po.xy) - po.z;
    if(d < 0)
      return texture(mobiles, i);
  }
  if(vsoTexCoord.y > 0.5 && vsoTexCoord.y < 0.505 && largeur == 0)
    return vec4(1);
  return vec4(largeur);
}

vec4 voronoif_ombre(void) {
  float mini = step / 2.0;
  vec3  po = texture(mobiles, step - mini).xyz;
  float mind = length(vsoTexCoord - po.xy), old_mind = 1, ombre;
  for(float i = mini + 2.0 * step; i < 1.0; i += 2.0 * step) {
    po = texture(mobiles, i + step).xyz;
    float d = length(vsoTexCoord - po.xy);
    if(mind > d) {
      old_mind = mind;
      mind = d;
      mini = i;
    }
  }
  ombre = pow(((old_mind - mind) / old_mind), 0.25);
  if(ombre < 0.3) ombre = 0;
  return ombre * texture(mobiles, mini);
} 

void main(void) {
  if(voronoi == 2)
    fragColor = voronoif_ombre();
  else
    fragColor = balle();
}
