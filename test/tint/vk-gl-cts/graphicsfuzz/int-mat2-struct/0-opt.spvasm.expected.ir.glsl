SKIP: FAILED

#version 310 es

struct S {
  int f1;
  mat2 f2;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  mat2 x_41 = mat2(vec2(0.0f), vec2(0.0f));
  int x_6 = 0;
  mat2 x_42 = mat2(vec2(0.0f), vec2(0.0f));
  mat2 x_49 = mat2(vec2(0.0f), vec2(0.0f));
  if ((tint_symbol.x < 0.0f)) {
    x_42 = mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f));
    x_49 = mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f));
  } else {
    x_42 = mat2(vec2(0.5f, -0.5f), vec2(-0.5f, 0.5f));
    x_49 = mat2(vec2(0.5f, -0.5f), vec2(-0.5f, 0.5f));
  }
  S x_51 = S(1, transpose(x_49));
  int x_52 = x_51.f1;
  x_6 = x_52;
  x_41 = x_51.f2;
  float v = float(x_52);
  float v_1 = (x_41[0u].x + x_41[1u].x);
  float v_2 = (x_41[0u].y + x_41[1u].y);
  x_GLF_color = vec4(v, v_1, v_2, float(x_52));
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
