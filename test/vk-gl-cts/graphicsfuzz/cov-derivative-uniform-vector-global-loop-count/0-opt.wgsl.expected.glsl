SKIP: FAILED

#version 310 es
precision mediump float;

struct tint_padded_array_element {
  float el;
};
struct buf1 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
};
struct tint_padded_array_element_1 {
  int el;
};
struct buf0 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[3];
};
struct buf2 {
  vec2 injectionSwitch;
};

int x_GLF_global_loop_count = 0;
layout (binding = 1) uniform buf1_1 {
  tint_padded_array_element x_GLF_uniform_float_values[2];
} x_7;
layout (binding = 0) uniform buf0_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[3];
} x_10;
layout (binding = 2) uniform buf2_1 {
  vec2 injectionSwitch;
} x_12;
vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  int r = 0;
  x_GLF_global_loop_count = 0;
  float x_42 = x_7.x_GLF_uniform_float_values[0].el;
  f = x_42;
  int x_44 = x_10.x_GLF_uniform_int_values[1].el;
  r = x_44;
  while (true) {
    int x_49 = r;
    int x_51 = x_10.x_GLF_uniform_int_values[2].el;
    if ((x_49 < x_51)) {
    } else {
      break;
    }
    x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
    vec2 x_57 = x_12.injectionSwitch;
    f = (f + ddx(x_57).y);
    {
      r = (r + 1);
    }
  }
  while (true) {
    if ((x_GLF_global_loop_count < 100)) {
    } else {
      break;
    }
    x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
    float x_74 = x_7.x_GLF_uniform_float_values[0].el;
    f = (f + x_74);
  }
  float x_77 = f;
  float x_79 = x_7.x_GLF_uniform_float_values[1].el;
  if ((x_77 == x_79)) {
    int x_85 = x_10.x_GLF_uniform_int_values[0].el;
    int x_88 = x_10.x_GLF_uniform_int_values[1].el;
    int x_91 = x_10.x_GLF_uniform_int_values[1].el;
    int x_94 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4(float(x_85), float(x_88), float(x_91), float(x_94));
  } else {
    int x_98 = x_10.x_GLF_uniform_int_values[1].el;
    float x_99 = float(x_98);
    x_GLF_color = vec4(x_99, x_99, x_99, x_99);
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
ERROR: 0:49: 'ddx' : no matching overloaded function found 
ERROR: 0:49: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



