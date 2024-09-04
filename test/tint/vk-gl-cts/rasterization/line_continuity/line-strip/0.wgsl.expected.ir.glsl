SKIP: FAILED

#version 310 es

struct main_out {
  vec4 color_1;
};
precision highp float;
precision highp int;


vec4 color = vec4(0.0f);
void main_1() {
  color = vec4(1.0f);
}
main_out main() {
  main_1();
  return main_out(color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
