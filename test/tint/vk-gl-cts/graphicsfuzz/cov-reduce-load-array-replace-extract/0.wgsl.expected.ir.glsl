SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;


struct buf0 {
  int zero;
};

struct main_out {
  vec4 x_GLF_color_1;
};

vec4 x_GLF_color = vec4(0.0f);
layout(binding = 0, std140)
uniform tint_symbol_2_1_ubo {
  buf0 tint_symbol_1;
} v;
layout(location = 0) out vec4 tint_symbol_loc0_Output;
void main_1() {
  int x_9[1] = int[1](0);
  int x_10_phi = 0;
  int x_33[1] = x_9;
  int x_6 = x_33[0u];
  {
    while(true) {
      x_GLF_color = vec4(0.0f);
      int x_7 = v.tint_symbol_1.zero;
      int x_8 = x_9[x_7];
      if ((x_8 == x_6)) {
        x_10_phi = 1;
        break;
      }
      x_10_phi = 2;
      break;
    }
  }
  int x_10 = x_10_phi;
  if (((x_10 == 1) | (x_10 == 2))) {
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
ERROR: 0:39: '|' :  wrong operand types: no operation '|' exists that takes a left-hand operand of type ' temp bool' and a right operand of type ' temp bool' (or there is no acceptable conversion)
ERROR: 0:39: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
