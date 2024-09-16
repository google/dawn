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
  vec4 v = vec4(0.0f);
  v = vec4(vec2(1.0f), v.zw);
  v = vec4(v.xy, vec2(2.0f));
  if (all((v == vec4(1.0f, 1.0f, 2.0f, 2.0f)))) {
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
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
