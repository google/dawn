SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct main_out {
  vec4 x_GLF_color_1;
};

vec4 x_GLF_color = vec4(0.0f);
layout(location = 0) out vec4 tint_symbol_loc0_Output;
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
main_out tint_symbol_inner() {
  main_1();
  return main_out(x_GLF_color);
}
void main() {
  tint_symbol_loc0_Output = tint_symbol_inner().x_GLF_color_1;
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'all' : no matching overloaded function found 
ERROR: 0:16: '=' :  cannot convert from ' const float' to ' temp bool'
ERROR: 0:16: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
