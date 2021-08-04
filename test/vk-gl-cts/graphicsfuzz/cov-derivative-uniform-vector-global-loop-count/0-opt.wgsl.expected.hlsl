static int x_GLF_global_loop_count = 0;
cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[2];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[3];
};
cbuffer cbuffer_x_12 : register(b2, space0) {
  uint4 x_12[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  int r = 0;
  x_GLF_global_loop_count = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_42 = asfloat(x_7[scalar_offset / 4][scalar_offset % 4]);
  f = x_42;
  const int x_44 = asint(x_10[1].x);
  r = x_44;
  while (true) {
    const int x_49 = r;
    const int x_51 = asint(x_10[2].x);
    if ((x_49 < x_51)) {
    } else {
      break;
    }
    x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
    const float2 x_57 = asfloat(x_12[0].xy);
    f = (f + ddx(x_57).y);
    {
      r = (r + 1);
    }
  }
  while (true) {
    if ((x_GLF_global_loop_count < 100)) {
    } else {
      break;
    }
    x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_74 = asfloat(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    f = (f + x_74);
  }
  const float x_77 = f;
  const float x_79 = asfloat(x_7[1].x);
  if ((x_77 == x_79)) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_85 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_88 = asint(x_10[1].x);
    const int x_91 = asint(x_10[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_94 = asint(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_85), float(x_88), float(x_91), float(x_94));
  } else {
    const int x_98 = asint(x_10[1].x);
    const float x_99 = float(x_98);
    x_GLF_color = float4(x_99, x_99, x_99, x_99);
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
