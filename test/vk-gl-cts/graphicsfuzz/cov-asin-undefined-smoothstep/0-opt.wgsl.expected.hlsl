cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[7];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 v2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 v3 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 v4 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_69 = false;
  bool x_77 = false;
  bool x_85 = false;
  bool x_93 = false;
  bool x_70_phi = false;
  bool x_78_phi = false;
  bool x_86_phi = false;
  bool x_94_phi = false;
  const float x_41 = asfloat(x_6[2].x);
  const float x_43 = asfloat(x_6[2].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_45 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const float x_47 = asfloat(x_6[2].x);
  v1 = float4(x_41, x_43, x_45, x_47);
  v2 = float4(1.570796371f, 1.119769573f, asfloat(0x7f800000u), 0.927295208f);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_50 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  v3 = float4(x_50, x_50, x_50, x_50);
  v4 = smoothstep(v1, v2, v3);
  const float4 x_56 = v4;
  x_GLF_color = float4(x_56.x, x_56.y, x_56.w, x_56.x);
  const float x_59 = v4.x;
  const float x_61 = asfloat(x_6[4].x);
  const bool x_62 = (x_59 > x_61);
  x_70_phi = x_62;
  if (x_62) {
    const float x_66 = v4.x;
    const float x_68 = asfloat(x_6[5].x);
    x_69 = (x_66 < x_68);
    x_70_phi = x_69;
  }
  const bool x_70 = x_70_phi;
  x_78_phi = x_70;
  if (x_70) {
    const float x_74 = v4.y;
    const float x_76 = asfloat(x_6[3].x);
    x_77 = (x_74 > x_76);
    x_78_phi = x_77;
  }
  const bool x_78 = x_78_phi;
  x_86_phi = x_78;
  if (x_78) {
    const float x_82 = v4.y;
    const float x_84 = asfloat(x_6[6].x);
    x_85 = (x_82 < x_84);
    x_86_phi = x_85;
  }
  const bool x_86 = x_86_phi;
  x_94_phi = x_86;
  if (x_86) {
    const float x_90 = v4.w;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_92 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_93 = (x_90 == x_92);
    x_94_phi = x_93;
  }
  if (x_94_phi) {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const float x_99 = asfloat(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_101 = asfloat(x_6[1].x);
    const float x_103 = asfloat(x_6[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_105 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_GLF_color = float4(x_99, x_101, x_103, x_105);
  } else {
    const float x_108 = asfloat(x_6[1].x);
    x_GLF_color = float4(x_108, x_108, x_108, x_108);
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
