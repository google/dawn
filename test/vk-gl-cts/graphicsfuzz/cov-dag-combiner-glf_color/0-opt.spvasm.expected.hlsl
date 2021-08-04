static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[4];
};
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[2];
};

float func_f1_(inout float b) {
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_90 = asfloat(x_7[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_92 = asfloat(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const float x_94 = asfloat(x_7[1].x);
  x_GLF_color = float4(x_90, x_92, x_94, 1.0f);
  x_GLF_color = x_GLF_color;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_98 = asfloat(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const float x_99 = b;
  if ((x_98 >= x_99)) {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const float x_104 = asfloat(x_7[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    return x_104;
  }
  const float x_106 = asfloat(x_7[2].x);
  return x_106;
}

void main_1() {
  float a = 0.0f;
  float param = 0.0f;
  float param_1 = 0.0f;
  bool x_71 = false;
  bool x_72_phi = false;
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const float x_44 = asfloat(x_7[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  param = x_44;
  const float x_45 = func_f1_(param);
  a = x_45;
  const uint scalar_offset_5 = ((16u * uint(0))) / 4;
  const float x_47 = asfloat(x_7[scalar_offset_5 / 4][scalar_offset_5 % 4]);
  const uint scalar_offset_6 = ((16u * uint(0))) / 4;
  const float x_49 = asfloat(x_7[scalar_offset_6 / 4][scalar_offset_6 % 4]);
  param_1 = (x_47 + x_49);
  const float x_51 = func_f1_(param_1);
  a = (a + x_51);
  const float x_54 = a;
  const float x_56 = asfloat(x_7[3].x);
  const bool x_57 = (x_54 == x_56);
  x_72_phi = x_57;
  if (x_57) {
    const float4 x_60 = x_GLF_color;
    const uint scalar_offset_7 = ((16u * uint(0))) / 4;
    const float x_62 = asfloat(x_7[scalar_offset_7 / 4][scalar_offset_7 % 4]);
    const uint scalar_offset_8 = ((16u * uint(0))) / 4;
    const float x_64 = asfloat(x_7[scalar_offset_8 / 4][scalar_offset_8 % 4]);
    const float x_66 = asfloat(x_7[1].x);
    const uint scalar_offset_9 = ((16u * uint(0))) / 4;
    const float x_68 = asfloat(x_7[scalar_offset_9 / 4][scalar_offset_9 % 4]);
    x_71 = all((x_60 == float4(x_62, x_64, x_66, x_68)));
    x_72_phi = x_71;
  }
  if (x_72_phi) {
    const uint scalar_offset_10 = ((16u * uint(0))) / 4;
    const int x_15 = asint(x_12[scalar_offset_10 / 4][scalar_offset_10 % 4]);
    const int x_16 = asint(x_12[1].x);
    const int x_17 = asint(x_12[1].x);
    const uint scalar_offset_11 = ((16u * uint(0))) / 4;
    const int x_18 = asint(x_12[scalar_offset_11 / 4][scalar_offset_11 % 4]);
    x_GLF_color = float4(float(x_15), float(x_16), float(x_17), float(x_18));
  } else {
    const int x_19 = asint(x_12[1].x);
    const float x_86 = float(x_19);
    x_GLF_color = float4(x_86, x_86, x_86, x_86);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
