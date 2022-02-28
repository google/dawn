vk-gl-cts/graphicsfuzz/cov-packhalf-unpackunorm/0-opt.wgsl:1:13 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr = @stride(16) array<f32, 4>;
            ^^^^^^

vk-gl-cts/graphicsfuzz/cov-packhalf-unpackunorm/0-opt.wgsl:7:15 warning: use of deprecated language feature: the @stride attribute is deprecated; use a larger type if necessary
type Arr_1 = @stride(16) array<i32, 4>;
              ^^^^^^

#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct tint_padded_array_element {
  float el;
};

struct buf0 {
  tint_padded_array_element x_GLF_uniform_float_values[4];
};

struct tint_padded_array_element_1 {
  int el;
};

struct buf1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
};

layout(binding = 0) uniform buf0_1 {
  tint_padded_array_element x_GLF_uniform_float_values[4];
} x_8;

layout(binding = 1) uniform buf1_1 {
  tint_padded_array_element_1 x_GLF_uniform_int_values[4];
} x_10;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  uint a = 0u;
  vec4 values = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec4 ref = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_85 = false;
  bool x_101 = false;
  bool x_117 = false;
  bool x_86_phi = false;
  bool x_102_phi = false;
  bool x_118_phi = false;
  a = packHalf2x16(vec2(1.0f, 1.0f));
  values = unpackUnorm4x8(a);
  float x_41 = x_8.x_GLF_uniform_float_values[3].el;
  float x_43 = x_8.x_GLF_uniform_float_values[1].el;
  float x_45 = x_8.x_GLF_uniform_float_values[0].el;
  float x_48 = x_8.x_GLF_uniform_float_values[3].el;
  float x_50 = x_8.x_GLF_uniform_float_values[0].el;
  float x_53 = x_8.x_GLF_uniform_float_values[1].el;
  float x_55 = x_8.x_GLF_uniform_float_values[0].el;
  ref = vec4(x_41, (x_43 / x_45), (x_48 / x_50), (x_53 / x_55));
  int x_59 = x_10.x_GLF_uniform_int_values[0].el;
  float x_61 = values[x_59];
  int x_63 = x_10.x_GLF_uniform_int_values[0].el;
  float x_65 = ref[x_63];
  float x_69 = x_8.x_GLF_uniform_float_values[2].el;
  bool x_70 = (abs((x_61 - x_65)) < x_69);
  x_86_phi = x_70;
  if (x_70) {
    int x_74 = x_10.x_GLF_uniform_int_values[1].el;
    float x_76 = values[x_74];
    int x_78 = x_10.x_GLF_uniform_int_values[1].el;
    float x_80 = ref[x_78];
    float x_84 = x_8.x_GLF_uniform_float_values[2].el;
    x_85 = (abs((x_76 - x_80)) < x_84);
    x_86_phi = x_85;
  }
  bool x_86 = x_86_phi;
  x_102_phi = x_86;
  if (x_86) {
    int x_90 = x_10.x_GLF_uniform_int_values[3].el;
    float x_92 = values[x_90];
    int x_94 = x_10.x_GLF_uniform_int_values[3].el;
    float x_96 = ref[x_94];
    float x_100 = x_8.x_GLF_uniform_float_values[2].el;
    x_101 = (abs((x_92 - x_96)) < x_100);
    x_102_phi = x_101;
  }
  bool x_102 = x_102_phi;
  x_118_phi = x_102;
  if (x_102) {
    int x_106 = x_10.x_GLF_uniform_int_values[2].el;
    float x_108 = values[x_106];
    int x_110 = x_10.x_GLF_uniform_int_values[2].el;
    float x_112 = ref[x_110];
    float x_116 = x_8.x_GLF_uniform_float_values[2].el;
    x_117 = (abs((x_108 - x_112)) < x_116);
    x_118_phi = x_117;
  }
  if (x_118_phi) {
    int x_123 = x_10.x_GLF_uniform_int_values[1].el;
    int x_126 = x_10.x_GLF_uniform_int_values[0].el;
    int x_129 = x_10.x_GLF_uniform_int_values[0].el;
    int x_132 = x_10.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4(float(x_123), float(x_126), float(x_129), float(x_132));
  } else {
    int x_136 = x_10.x_GLF_uniform_int_values[0].el;
    float x_137 = float(x_136);
    x_GLF_color = vec4(x_137, x_137, x_137, x_137);
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
