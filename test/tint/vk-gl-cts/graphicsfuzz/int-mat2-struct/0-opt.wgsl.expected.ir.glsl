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
  mat2 x_49_phi = mat2(vec2(0.0f), vec2(0.0f));
  float x_44 = tint_symbol.x;
  if ((x_44 < 0.0f)) {
    x_42 = mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f));
    x_49_phi = mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f));
  } else {
    x_42 = mat2(vec2(0.5f, -0.5f), vec2(-0.5f, 0.5f));
    x_49_phi = mat2(vec2(0.5f, -0.5f), vec2(-0.5f, 0.5f));
  }
  mat2 x_49 = x_49_phi;
  S x_51 = S(1, transpose(x_49));
  int x_52 = x_51.f1;
  x_6 = x_52;
  x_41 = x_51.f2;
  mat2 x_56 = x_41;
  mat2 x_59 = x_41;
  mat2 x_63 = x_41;
  mat2 x_66 = x_41;
  float v = float(x_52);
  x_GLF_color = vec4(v, (x_56[0u][0u] + x_59[1u][0u]), (x_63[0u][1u] + x_66[1u][1u]), float(x_52));
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
