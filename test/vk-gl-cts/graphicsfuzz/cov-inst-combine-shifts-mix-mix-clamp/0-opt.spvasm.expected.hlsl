cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int A[2] = (int[2])0;
  int i = 0;
  int a = 0;
  float2 v1 = float2(0.0f, 0.0f);
  float2 v2 = float2(0.0f, 0.0f);
  int b = 0;
  const int x_46 = asint(x_6[2].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_48 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  A[x_46] = x_48;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_51 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const int x_53 = asint(x_6[1].x);
  A[x_51] = x_53;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_56 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  i = x_56;
  while (true) {
    const int x_61 = i;
    const int x_63 = asint(x_6[2].x);
    if ((x_61 > x_63)) {
    } else {
      break;
    }
    i = (i - 1);
  }
  const float x_69 = asfloat(x_10[1].x);
  const float x_71 = asfloat(x_10[1].x);
  const int x_76 = A[((x_69 >= x_71) ? 1 : i)];
  a = x_76;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_78 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const int x_80 = a;
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const int x_84 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const uint scalar_offset_5 = ((16u * uint(0))) / 4;
  const int x_87 = asint(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
  const float x_91 = asfloat(x_10[1].x);
  const uint scalar_offset_6 = ((16u * uint(0))) / 4;
  const float x_93 = asfloat(x_10[scalar_offset_6 / 4][scalar_offset_6 % 4]);
  v1 = (bool2((x_91 < x_93), true) ? float2(float(x_84), float(x_87)) : float2(float(x_78), float(x_80)));
  const int x_98 = asint(x_6[2].x);
  const float x_100 = v1[x_98];
  const uint scalar_offset_7 = ((16u * uint(0))) / 4;
  const int x_103 = asint(x_6[scalar_offset_7 / 4][scalar_offset_7 % 4]);
  const float x_105 = v1[x_103];
  v2 = (bool2(false, false) ? float2(x_105, x_105) : float2(x_100, x_100));
  const int x_109 = asint(x_6[1].x);
  const float x_110 = float(x_109);
  const uint scalar_offset_8 = ((16u * uint(0))) / 4;
  const int x_113 = asint(x_6[scalar_offset_8 / 4][scalar_offset_8 % 4]);
  const float x_114 = float(x_113);
  const int x_121 = A[int(clamp(float2(x_110, x_110), float2(x_114, x_114), v2).x)];
  b = x_121;
  const int x_122 = b;
  const int x_124 = asint(x_6[1].x);
  if ((x_122 == x_124)) {
    const uint scalar_offset_9 = ((16u * uint(0))) / 4;
    const int x_130 = asint(x_6[scalar_offset_9 / 4][scalar_offset_9 % 4]);
    const int x_133 = asint(x_6[2].x);
    const int x_136 = asint(x_6[2].x);
    const uint scalar_offset_10 = ((16u * uint(0))) / 4;
    const int x_139 = asint(x_6[scalar_offset_10 / 4][scalar_offset_10 % 4]);
    x_GLF_color = float4(float(x_130), float(x_133), float(x_136), float(x_139));
  } else {
    const int x_143 = asint(x_6[2].x);
    const float x_144 = float(x_143);
    x_GLF_color = float4(x_144, x_144, x_144, x_144);
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
