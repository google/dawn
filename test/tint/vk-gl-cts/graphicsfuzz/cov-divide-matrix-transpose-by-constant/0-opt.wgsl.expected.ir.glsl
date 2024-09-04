SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  mat2 m = mat2(vec2(0.0f), vec2(0.0f));
  m = mat2(vec2(0.5f, 1.5f), vec2(1.0f, 2.0f));
  mat2 x_33 = m;
  bool v = all((x_33[0u] == vec2(0.5f, 1.5f)));
  if ((v & all((x_33[1u] == vec2(1.0f, 2.0f))))) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
