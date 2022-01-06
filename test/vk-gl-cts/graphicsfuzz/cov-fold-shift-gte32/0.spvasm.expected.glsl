SKIP: FAILED

#version 310 es
precision mediump float;

struct buf0 {
  uint one;
};

layout (binding = 0) uniform buf0_1 {
  uint one;
} x_6;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  uint a = 0u;
  uint b = 0u;
  uint c = 0u;
  uint d = 0u;
  uint e = 0u;
  uint f = 0u;
  uint x_41 = x_6.one;
  a = ((77u + x_41) >> 32u);
  uint x_45 = x_6.one;
  b = ((3243u + x_45) >> 33u);
  uint x_49 = x_6.one;
  c = ((23u + x_49) >> 345u);
  uint x_53 = x_6.one;
  d = ((2395u + x_53) << 32u);
  uint x_57 = x_6.one;
  e = ((290485u + x_57) << 33u);
  uint x_61 = x_6.one;
  f = ((44321u + x_61) << 345u);
  if ((a != 1u)) {
    a = 1u;
  }
  if ((b != 0u)) {
    b = 0u;
  }
  if ((c != 1u)) {
    c = 1u;
  }
  if ((d != 0u)) {
    d = 0u;
  }
  if ((e != 1u)) {
    e = 1u;
  }
  if ((f != 0u)) {
    f = 0u;
  }
  if (((((((a == 1u) & (b == 0u)) & (c == 1u)) & (d == 0u)) & (e == 1u)) & (f == 0u))) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};
struct tint_symbol_1 {
  vec4 x_GLF_color_1;
};

main_out tint_symbol_inner() {
  main_1();
  main_out tint_symbol_2 = main_out(x_GLF_color);
  return tint_symbol_2;
}

tint_symbol_1 tint_symbol() {
  main_out inner_result = tint_symbol_inner();
  tint_symbol_1 wrapper_result = tint_symbol_1(vec4(0.0f, 0.0f, 0.0f, 0.0f));
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
out vec4 x_GLF_color_1;
void main() {
  tint_symbol_1 outputs;
  outputs = tint_symbol();
  x_GLF_color_1 = outputs.x_GLF_color_1;
}


Error parsing GLSL shader:
ERROR: 0:50: '&' :  wrong operand types: no operation '&' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:50: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



