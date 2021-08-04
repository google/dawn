float4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const int x_30 = asint(x_6[1].x);
  i = x_30;
  while (true) {
    const int x_35 = i;
    const int x_37 = asint(x_6[2].x);
    if ((x_35 < x_37)) {
    } else {
      break;
    }
    v = tint_unpack4x8unorm(100u);
    const float x_42 = v.x;
    if ((int(x_42) > i)) {
      const int x_49 = asint(x_6[1].x);
      const float x_50 = float(x_49);
      x_GLF_color = float4(x_50, x_50, x_50, x_50);
      return;
    }
    {
      i = (i + 1);
    }
  }
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_55 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const int x_58 = asint(x_6[1].x);
  const int x_61 = asint(x_6[1].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_64 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  x_GLF_color = float4(float(x_55), float(x_58), float(x_61), float(x_64));
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
