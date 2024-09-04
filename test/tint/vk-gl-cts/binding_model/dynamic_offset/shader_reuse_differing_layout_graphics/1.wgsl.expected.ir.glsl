SKIP: FAILED

#version 310 es

struct main_out {
  vec4 final_color_1;
};
precision highp float;
precision highp int;


vec4 final_color = vec4(0.0f);
vec4 frag_color = vec4(0.0f);
void main_1() {
  vec4 x_12 = frag_color;
  final_color = x_12;
}
main_out main(vec4 frag_color_param) {
  frag_color = frag_color_param;
  main_1();
  return main_out(final_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
