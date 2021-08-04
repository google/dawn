void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[5];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int i = 0;
  const int x_40 = asint(x_6[1].x);
  const int x_43 = asint(x_6[2].x);
  const int x_46 = asint(x_6[3].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_49 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  v = float4(float(x_40), float(x_43), float(x_46), float(x_49));
  const int x_53 = asint(x_6[4].x);
  i = x_53;
  while (true) {
    const int x_58 = i;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_60 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    if ((x_58 < x_60)) {
    } else {
      break;
    }
    const float4 x_63 = v;
    const float4 x_64 = v;
    const float4 x_65 = v;
    const float4 x_66 = v;
    const int x_88 = i;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_92 = asfloat(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if ((float4x4(float4(x_63.x, x_63.y, x_63.z, x_63.w), float4(x_64.x, x_64.y, x_64.z, x_64.w), float4(x_65.x, x_65.y, x_65.z, x_65.w), float4(x_66.x, x_66.y, x_66.z, x_66.w))[0u][x_88] > x_92)) {
      const int x_96 = i;
      const float4 x_97 = v;
      const float x_99 = asfloat(x_9[1].x);
      const uint scalar_offset_3 = ((16u * uint(0))) / 4;
      const float x_102 = asfloat(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
      const int x_106 = asint(x_6[1].x);
      set_float4(v, x_96, clamp(x_97, float4(x_99, x_99, x_99, x_99), float4(x_102, x_102, x_102, x_102))[x_106]);
    }
    {
      i = (i + 1);
    }
  }
  const float4 x_111 = v;
  const int x_113 = asint(x_6[1].x);
  const float x_114 = float(x_113);
  if (all((x_111 == float4(x_114, x_114, x_114, x_114)))) {
    const int x_122 = asint(x_6[1].x);
    const int x_125 = asint(x_6[4].x);
    const int x_128 = asint(x_6[4].x);
    const int x_131 = asint(x_6[1].x);
    x_GLF_color = float4(float(x_122), float(x_125), float(x_128), float(x_131));
  } else {
    const int x_135 = asint(x_6[4].x);
    const float x_136 = float(x_135);
    x_GLF_color = float4(x_136, x_136, x_136, x_136);
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
