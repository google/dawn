static int x_GLF_global_loop_count = 0;
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};

void main_1() {
  x_GLF_global_loop_count = 0;
  while (true) {
    if ((x_GLF_global_loop_count < 100)) {
    } else {
      break;
    }
    x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
    if (((x_GLF_global_loop_count * x_GLF_global_loop_count) > 10)) {
      break;
    }
  }
  if ((x_GLF_global_loop_count == 4)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_47 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    const int x_50 = asint(x_6[1].x);
    const int x_53 = asint(x_6[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_56 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_GLF_color = float4(float(x_47), float(x_50), float(x_53), float(x_56));
  } else {
    const int x_60 = asint(x_6[1].x);
    const float x_61 = float(x_60);
    x_GLF_color = float4(x_61, x_61, x_61, x_61);
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
