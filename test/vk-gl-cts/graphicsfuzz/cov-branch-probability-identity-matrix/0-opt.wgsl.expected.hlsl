cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[4];
};
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float sums[2] = (float[2])0;
  int a = 0;
  int b = 0;
  int c = 0;
  int d = 0;
  float2x2 indexable = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 indexable_1 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_158 = false;
  bool x_159_phi = false;
  const int x_16 = asint(x_6[1].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_85 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
  sums[x_16] = -(x_85);
  const int x_18 = asint(x_6[2].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_90 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  sums[x_18] = -(x_90);
  const int x_19 = asint(x_6[1].x);
  a = x_19;
  while (true) {
    const int x_20 = a;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_21 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if ((x_20 < x_21)) {
    } else {
      break;
    }
    const int x_22 = asint(x_6[1].x);
    b = x_22;
    while (true) {
      const int x_23 = b;
      const int x_24 = asint(x_6[3].x);
      if ((x_23 < x_24)) {
      } else {
        break;
      }
      const int x_25 = asint(x_6[1].x);
      c = x_25;
      {
        for(; (c <= a); c = (c + 1)) {
          const int x_28 = asint(x_6[1].x);
          d = x_28;
          while (true) {
            const int x_29 = d;
            const int x_30 = asint(x_6[3].x);
            if ((x_29 < x_30)) {
            } else {
              break;
            }
            const int x_31 = a;
            const int x_32 = asint(x_6[2].x);
            const float x_125 = float(x_32);
            const int x_33 = c;
            const int x_34 = asint(x_6[2].x);
            indexable = float2x2(float2(x_125, 0.0f), float2(0.0f, x_125));
            const float x_131 = indexable[x_33][x_34];
            sums[x_31] = x_131;
            const int x_35 = a;
            const int x_36 = asint(x_6[2].x);
            const float x_134 = float(x_36);
            const int x_37 = c;
            const int x_38 = asint(x_6[2].x);
            indexable_1 = float2x2(float2(x_134, 0.0f), float2(0.0f, x_134));
            const float x_140 = indexable_1[x_37][x_38];
            const float x_142 = sums[x_35];
            sums[x_35] = (x_142 + x_140);
            {
              d = (d + 1);
            }
          }
        }
      }
      {
        b = (b + 1);
      }
    }
    {
      a = (a + 1);
    }
  }
  const int x_47 = asint(x_6[1].x);
  const float x_147 = sums[x_47];
  const float x_149 = asfloat(x_8[1].x);
  const bool x_150 = (x_147 == x_149);
  x_159_phi = x_150;
  if (x_150) {
    const int x_48 = asint(x_6[2].x);
    const float x_155 = sums[x_48];
    const float x_157 = asfloat(x_8[2].x);
    x_158 = (x_155 == x_157);
    x_159_phi = x_158;
  }
  if (x_159_phi) {
    const int x_49 = asint(x_6[2].x);
    const int x_50 = asint(x_6[1].x);
    const int x_51 = asint(x_6[1].x);
    const int x_52 = asint(x_6[2].x);
    x_GLF_color = float4(float(x_49), float(x_50), float(x_51), float(x_52));
  } else {
    const int x_53 = asint(x_6[1].x);
    const float x_173 = float(x_53);
    x_GLF_color = float4(x_173, x_173, x_173, x_173);
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
