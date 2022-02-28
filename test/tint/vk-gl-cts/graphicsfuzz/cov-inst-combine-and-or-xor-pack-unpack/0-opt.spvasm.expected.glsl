#version 310 es
precision mediump float;

layout(location = 0) out vec4 x_GLF_color_1_1;
struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

layout(binding = 0) uniform buf0_1 {
  strided_arr x_GLF_uniform_float_values[2];
} x_6;

layout(binding = 1) uniform buf1_1 {
  strided_arr_1 x_GLF_uniform_int_values[4];
} x_10;

vec4 x_GLF_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
void main_1() {
  uint a = 0u;
  vec4 v1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec4 ref = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_85 = false;
  bool x_97 = false;
  bool x_109 = false;
  bool x_86_phi = false;
  bool x_98_phi = false;
  bool x_110_phi = false;
  float x_36 = x_6.x_GLF_uniform_float_values[0].el;
  a = packUnorm4x8(vec4(x_36, x_36, x_36, x_36));
  v1 = unpackSnorm4x8(a);
  float x_42 = x_6.x_GLF_uniform_float_values[0].el;
  float x_45 = x_6.x_GLF_uniform_float_values[1].el;
  float x_48 = x_6.x_GLF_uniform_float_values[0].el;
  float x_51 = x_6.x_GLF_uniform_float_values[1].el;
  float x_54 = x_6.x_GLF_uniform_float_values[0].el;
  float x_57 = x_6.x_GLF_uniform_float_values[1].el;
  float x_60 = x_6.x_GLF_uniform_float_values[0].el;
  float x_63 = x_6.x_GLF_uniform_float_values[1].el;
  ref = vec4((-(x_42) / x_45), (-(x_48) / x_51), (-(x_54) / x_57), (-(x_60) / x_63));
  int x_67 = x_10.x_GLF_uniform_int_values[1].el;
  float x_69 = v1[x_67];
  int x_71 = x_10.x_GLF_uniform_int_values[0].el;
  float x_73 = ref[x_71];
  bool x_74 = (x_69 == x_73);
  x_86_phi = x_74;
  if (x_74) {
    int x_78 = x_10.x_GLF_uniform_int_values[3].el;
    float x_80 = v1[x_78];
    int x_82 = x_10.x_GLF_uniform_int_values[2].el;
    float x_84 = ref[x_82];
    x_85 = (x_80 == x_84);
    x_86_phi = x_85;
  }
  bool x_86 = x_86_phi;
  x_98_phi = x_86;
  if (x_86) {
    int x_90 = x_10.x_GLF_uniform_int_values[2].el;
    float x_92 = v1[x_90];
    int x_94 = x_10.x_GLF_uniform_int_values[3].el;
    float x_96 = ref[x_94];
    x_97 = (x_92 == x_96);
    x_98_phi = x_97;
  }
  bool x_98 = x_98_phi;
  x_110_phi = x_98;
  if (x_98) {
    int x_102 = x_10.x_GLF_uniform_int_values[0].el;
    float x_104 = v1[x_102];
    int x_106 = x_10.x_GLF_uniform_int_values[1].el;
    float x_108 = ref[x_106];
    x_109 = (x_104 == x_108);
    x_110_phi = x_109;
  }
  if (x_110_phi) {
    int x_115 = x_10.x_GLF_uniform_int_values[3].el;
    int x_118 = x_10.x_GLF_uniform_int_values[1].el;
    int x_121 = x_10.x_GLF_uniform_int_values[1].el;
    int x_124 = x_10.x_GLF_uniform_int_values[3].el;
    x_GLF_color = vec4(float(x_115), float(x_118), float(x_121), float(x_124));
  } else {
    int x_128 = x_10.x_GLF_uniform_int_values[1].el;
    float x_130 = v1[x_128];
    x_GLF_color = vec4(x_130, x_130, x_130, x_130);
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
