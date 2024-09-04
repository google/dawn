SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
vec3 f_mf22_(inout mat2 m) {
  {
    while(true) {
      return vec3(1.0f);
    }
  }
  /* unreachable */
}
void main_1() {
  mat2 param = mat2(vec2(0.0f), vec2(0.0f));
  mat2 x_38_phi = mat2(vec2(0.0f), vec2(0.0f));
  float x_34 = tint_symbol.x;
  x_38_phi = mat2(vec2(0.0f), vec2(0.0f));
  if ((x_34 >= 0.0f)) {
    x_38_phi = mat2(vec2(1.0f, 0.0f), vec2(0.0f, 1.0f));
  }
  mat2 x_38 = x_38_phi;
  param = (x_38 * x_38);
  vec3 x_40 = f_mf22_(param);
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
