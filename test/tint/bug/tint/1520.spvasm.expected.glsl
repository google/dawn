#version 310 es
precision mediump float;

layout(location = 0) in vec4 vcolor_S0_param_1;
layout(location = 0) out vec4 sk_FragColor_1_1;
struct UniformBuffer {
  float unknownInput_S1_c0;
  vec4 ucolorRed_S1_c0;
  vec4 ucolorGreen_S1_c0;
  mat3 umatrix_S1;
};

layout(binding = 0) uniform UniformBuffer_1 {
  float unknownInput_S1_c0;
  vec4 ucolorRed_S1_c0;
  vec4 ucolorGreen_S1_c0;
  mat3 umatrix_S1;
} x_4;

vec4 sk_FragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
bool sk_Clockwise = false;
vec4 vcolor_S0 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
bool test_int_S1_c0_b() {
  int unknown = 0;
  bool ok = false;
  ivec4 val = ivec4(0, 0, 0, 0);
  bool x_40 = false;
  bool x_54 = false;
  bool x_65 = false;
  bool x_41_phi = false;
  bool x_55_phi = false;
  bool x_66_phi = false;
  float x_26 = x_4.unknownInput_S1_c0;
  int x_27 = int(x_26);
  unknown = x_27;
  ok = true;
  x_41_phi = false;
  if (true) {
    x_40 = all(equal((ivec4(0, 0, 0, 0) / ivec4(x_27, x_27, x_27, x_27)), ivec4(0, 0, 0, 0)));
    x_41_phi = x_40;
  }
  bool x_41 = x_41_phi;
  ok = x_41;
  ivec4 x_44 = ivec4(x_27, x_27, x_27, x_27);
  val = x_44;
  ivec4 x_47 = (x_44 + ivec4(1, 1, 1, 1));
  val = x_47;
  ivec4 x_48 = (x_47 - ivec4(1, 1, 1, 1));
  val = x_48;
  ivec4 x_49 = (x_48 + ivec4(1, 1, 1, 1));
  val = x_49;
  ivec4 x_50 = (x_49 - ivec4(1, 1, 1, 1));
  val = x_50;
  x_55_phi = false;
  if (x_41) {
    x_54 = all(equal(x_50, x_44));
    x_55_phi = x_54;
  }
  bool x_55 = x_55_phi;
  ok = x_55;
  ivec4 x_58 = (x_50 * ivec4(2, 2, 2, 2));
  val = x_58;
  ivec4 x_59 = (x_58 / ivec4(2, 2, 2, 2));
  val = x_59;
  ivec4 x_60 = (x_59 * ivec4(2, 2, 2, 2));
  val = x_60;
  ivec4 x_61 = (x_60 / ivec4(2, 2, 2, 2));
  val = x_61;
  x_66_phi = false;
  if (x_55) {
    x_65 = all(equal(x_61, x_44));
    x_66_phi = x_65;
  }
  bool x_66 = x_66_phi;
  ok = x_66;
  return x_66;
}

void main_1() {
  vec4 outputColor_S0 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec4 output_S1 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  float x_8_unknown = 0.0f;
  bool x_9_ok = false;
  vec4 x_10_val = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  vec4 x_116 = vec4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_86 = false;
  bool x_99 = false;
  bool x_110 = false;
  bool x_114 = false;
  bool x_87_phi = false;
  bool x_100_phi = false;
  bool x_111_phi = false;
  bool x_115_phi = false;
  outputColor_S0 = vcolor_S0;
  float x_77 = x_4.unknownInput_S1_c0;
  x_8_unknown = x_77;
  x_9_ok = true;
  x_87_phi = false;
  if (true) {
    x_86 = all(equal((vec4(0.0f, 0.0f, 0.0f, 0.0f) / vec4(x_77, x_77, x_77, x_77)), vec4(0.0f, 0.0f, 0.0f, 0.0f)));
    x_87_phi = x_86;
  }
  bool x_87 = x_87_phi;
  x_9_ok = x_87;
  vec4 x_89 = vec4(x_77, x_77, x_77, x_77);
  x_10_val = x_89;
  vec4 x_92 = (x_89 + vec4(1.0f, 1.0f, 1.0f, 1.0f));
  x_10_val = x_92;
  vec4 x_93 = (x_92 - vec4(1.0f, 1.0f, 1.0f, 1.0f));
  x_10_val = x_93;
  vec4 x_94 = (x_93 + vec4(1.0f, 1.0f, 1.0f, 1.0f));
  x_10_val = x_94;
  vec4 x_95 = (x_94 - vec4(1.0f, 1.0f, 1.0f, 1.0f));
  x_10_val = x_95;
  x_100_phi = false;
  if (x_87) {
    x_99 = all(equal(x_95, x_89));
    x_100_phi = x_99;
  }
  bool x_100 = x_100_phi;
  x_9_ok = x_100;
  vec4 x_103 = (x_95 * vec4(2.0f, 2.0f, 2.0f, 2.0f));
  x_10_val = x_103;
  vec4 x_104 = (x_103 / vec4(2.0f, 2.0f, 2.0f, 2.0f));
  x_10_val = x_104;
  vec4 x_105 = (x_104 * vec4(2.0f, 2.0f, 2.0f, 2.0f));
  x_10_val = x_105;
  vec4 x_106 = (x_105 / vec4(2.0f, 2.0f, 2.0f, 2.0f));
  x_10_val = x_106;
  x_111_phi = false;
  if (x_100) {
    x_110 = all(equal(x_106, x_89));
    x_111_phi = x_110;
  }
  bool x_111 = x_111_phi;
  x_9_ok = x_111;
  x_115_phi = false;
  if (x_111) {
    x_114 = test_int_S1_c0_b();
    x_115_phi = x_114;
  }
  if (x_115_phi) {
    vec4 x_122 = x_4.ucolorGreen_S1_c0;
    x_116 = x_122;
  } else {
    vec4 x_124 = x_4.ucolorRed_S1_c0;
    x_116 = x_124;
  }
  vec4 x_125 = x_116;
  output_S1 = x_125;
  sk_FragColor = x_125;
  return;
}

struct main_out {
  vec4 sk_FragColor_1;
};

main_out tint_symbol(bool sk_Clockwise_param, vec4 vcolor_S0_param) {
  sk_Clockwise = sk_Clockwise_param;
  vcolor_S0 = vcolor_S0_param;
  main_1();
  main_out tint_symbol_1 = main_out(sk_FragColor);
  return tint_symbol_1;
}

void main() {
  main_out inner_result = tint_symbol(gl_FrontFacing, vcolor_S0_param_1);
  sk_FragColor_1_1 = inner_result.sk_FragColor_1;
  return;
}
