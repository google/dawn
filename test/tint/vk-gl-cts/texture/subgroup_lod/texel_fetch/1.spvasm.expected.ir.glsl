SKIP: FAILED

#version 310 es

struct main_out {
  vec4 color_out_1;
};
precision highp float;
precision highp int;


vec4 color_out = vec4(0.0f);
vec4 color_in = vec4(0.0f);
void main_1() {
  color_out = color_in;
}
main_out main(vec4 color_in_param) {
  color_in = color_in_param;
  main_1();
  return main_out(color_out);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
