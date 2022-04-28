int4 value_or_one_if_zero_int4(int4 value) {
  return value == int4(0, 0, 0, 0) ? int4(1, 1, 1, 1) : value;
}

cbuffer cbuffer_x_4 : register(b0, space0) {
  uint4 x_4[7];
};
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static bool sk_Clockwise = false;
static float4 vcolor_S0 = float4(0.0f, 0.0f, 0.0f, 0.0f);

bool test_int_S1_c0_b() {
  int unknown = 0;
  bool ok = false;
  int4 val = int4(0, 0, 0, 0);
  bool x_40 = false;
  bool x_54 = false;
  bool x_65 = false;
  bool x_41_phi = false;
  bool x_55_phi = false;
  bool x_66_phi = false;
  const float x_26 = asfloat(x_4[1].x);
  const int x_27 = int(x_26);
  unknown = x_27;
  ok = true;
  x_41_phi = false;
  if (true) {
    x_40 = all(((int4(0, 0, 0, 0) / value_or_one_if_zero_int4(int4(x_27, x_27, x_27, x_27))) == int4(0, 0, 0, 0)));
    x_41_phi = x_40;
  }
  const bool x_41 = x_41_phi;
  ok = x_41;
  const int4 x_44 = int4(x_27, x_27, x_27, x_27);
  val = x_44;
  const int4 x_47 = (x_44 + int4(1, 1, 1, 1));
  val = x_47;
  const int4 x_48 = (x_47 - int4(1, 1, 1, 1));
  val = x_48;
  const int4 x_49 = (x_48 + int4(1, 1, 1, 1));
  val = x_49;
  const int4 x_50 = (x_49 - int4(1, 1, 1, 1));
  val = x_50;
  x_55_phi = false;
  if (x_41) {
    x_54 = all((x_50 == x_44));
    x_55_phi = x_54;
  }
  const bool x_55 = x_55_phi;
  ok = x_55;
  const int4 x_58 = (x_50 * int4(2, 2, 2, 2));
  val = x_58;
  const int4 x_59 = (x_58 / int4(2, 2, 2, 2));
  val = x_59;
  const int4 x_60 = (x_59 * int4(2, 2, 2, 2));
  val = x_60;
  const int4 x_61 = (x_60 / int4(2, 2, 2, 2));
  val = x_61;
  x_66_phi = false;
  if (x_55) {
    x_65 = all((x_61 == x_44));
    x_66_phi = x_65;
  }
  const bool x_66 = x_66_phi;
  ok = x_66;
  return x_66;
}

void main_1() {
  float4 outputColor_S0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 output_S1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float x_8_unknown = 0.0f;
  bool x_9_ok = false;
  float4 x_10_val = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_116 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_86 = false;
  bool x_99 = false;
  bool x_110 = false;
  bool x_114 = false;
  bool x_87_phi = false;
  bool x_100_phi = false;
  bool x_111_phi = false;
  bool x_115_phi = false;
  outputColor_S0 = vcolor_S0;
  const float x_77 = asfloat(x_4[1].x);
  x_8_unknown = x_77;
  x_9_ok = true;
  x_87_phi = false;
  if (true) {
    x_86 = all(((float4(0.0f, 0.0f, 0.0f, 0.0f) / float4(x_77, x_77, x_77, x_77)) == float4(0.0f, 0.0f, 0.0f, 0.0f)));
    x_87_phi = x_86;
  }
  const bool x_87 = x_87_phi;
  x_9_ok = x_87;
  const float4 x_89 = float4(x_77, x_77, x_77, x_77);
  x_10_val = x_89;
  const float4 x_92 = (x_89 + float4(1.0f, 1.0f, 1.0f, 1.0f));
  x_10_val = x_92;
  const float4 x_93 = (x_92 - float4(1.0f, 1.0f, 1.0f, 1.0f));
  x_10_val = x_93;
  const float4 x_94 = (x_93 + float4(1.0f, 1.0f, 1.0f, 1.0f));
  x_10_val = x_94;
  const float4 x_95 = (x_94 - float4(1.0f, 1.0f, 1.0f, 1.0f));
  x_10_val = x_95;
  x_100_phi = false;
  if (x_87) {
    x_99 = all((x_95 == x_89));
    x_100_phi = x_99;
  }
  const bool x_100 = x_100_phi;
  x_9_ok = x_100;
  const float4 x_103 = (x_95 * float4(2.0f, 2.0f, 2.0f, 2.0f));
  x_10_val = x_103;
  const float4 x_104 = (x_103 / float4(2.0f, 2.0f, 2.0f, 2.0f));
  x_10_val = x_104;
  const float4 x_105 = (x_104 * float4(2.0f, 2.0f, 2.0f, 2.0f));
  x_10_val = x_105;
  const float4 x_106 = (x_105 / float4(2.0f, 2.0f, 2.0f, 2.0f));
  x_10_val = x_106;
  x_111_phi = false;
  if (x_100) {
    x_110 = all((x_106 == x_89));
    x_111_phi = x_110;
  }
  const bool x_111 = x_111_phi;
  x_9_ok = x_111;
  x_115_phi = false;
  if (x_111) {
    x_114 = test_int_S1_c0_b();
    x_115_phi = x_114;
  }
  if (x_115_phi) {
    const float4 x_122 = asfloat(x_4[3]);
    x_116 = x_122;
  } else {
    const float4 x_124 = asfloat(x_4[2]);
    x_116 = x_124;
  }
  const float4 x_125 = x_116;
  output_S1 = x_125;
  sk_FragColor = x_125;
  return;
}

struct main_out {
  float4 sk_FragColor_1;
};
struct tint_symbol_1 {
  float4 vcolor_S0_param : TEXCOORD0;
  bool sk_Clockwise_param : SV_IsFrontFace;
};
struct tint_symbol_2 {
  float4 sk_FragColor_1 : SV_Target0;
};

main_out main_inner(bool sk_Clockwise_param, float4 vcolor_S0_param) {
  sk_Clockwise = sk_Clockwise_param;
  vcolor_S0 = vcolor_S0_param;
  main_1();
  const main_out tint_symbol_5 = {sk_FragColor};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param, tint_symbol.vcolor_S0_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
