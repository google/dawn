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
  m = mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f));
  mat2 x_26 = m;
  mat2 x_28 = m;
  mat2 v = transpose(x_26);
  mat2 x_30 = (v * transpose(x_28));
  mat2 x_31 = m;
  mat2 x_32 = m;
  mat2 x_34 = transpose((x_31 * x_32));
  bool v_1 = all((x_30[0u] == x_34[0u]));
  if ((v_1 & all((x_30[1u] == x_34[1u])))) {
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
