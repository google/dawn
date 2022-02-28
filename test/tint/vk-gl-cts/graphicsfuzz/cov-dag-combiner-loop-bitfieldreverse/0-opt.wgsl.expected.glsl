vk-gl-cts/graphicsfuzz/cov-dag-combiner-loop-bitfieldreverse/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<i32, 4>;
            ^^^^^^

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  int el;
};

struct buf0 {
  tint_padded_array_element x_GLF_uniform_int_values[4];
};

layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_int_values[4];
} x_6;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  int a = 0;
  int i = 0;
  int x_27 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_27;
  int x_29 = x_6.x_GLF_uniform_int_values[3].el;
  i = -(x_29);
  while (true) {
    int x_36 = (i + 1);
    i = x_36;
    int x_39 = x_6.x_GLF_uniform_int_values[2].el;
    if ((bitfieldReverse(x_36) <= x_39)) {
    } else {
      break;
    }
    a = (a + 1);
  }
  int x_44 = a;
  int x_46 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_44 == x_46)) {
    int x_52 = x_6.x_GLF_uniform_int_values[2].el;
    int x_55 = x_6.x_GLF_uniform_int_values[1].el;
    int x_58 = x_6.x_GLF_uniform_int_values[1].el;
    int x_61 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4(float(x_52), float(x_55), float(x_58), float(x_61));
  } else {
    int x_65 = x_6.x_GLF_uniform_int_values[1].el;
    float x_66 = float(x_65);
    x_GLF_color = vec4(x_66, x_66, x_66, x_66);
  }
  return;
}

struct main_out {
  vec4 x_GLF_color_1;
};

main_out tint_symbol() {
  main_1();
  main_out tint_symbol_1 = main_out(x_GLF_color);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol();
  x_GLF_color_1_1 = inner_result.x_GLF_color_1;
  return;
}
