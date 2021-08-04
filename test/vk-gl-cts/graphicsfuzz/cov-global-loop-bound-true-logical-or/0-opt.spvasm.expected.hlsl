static int x_GLF_global_loop_count = 0;
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};

void main_1() {
  x_GLF_global_loop_count = 0;
  const int x_26 = asint(x_6[1].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_29 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_32 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const int x_35 = asint(x_6[1].x);
  x_GLF_color = float4(float(x_26), float(x_29), float(x_32), float(x_35));
  while (true) {
    bool x_54 = false;
    bool x_55_phi = false;
    if ((x_GLF_global_loop_count < 100)) {
    } else {
      break;
    }
    x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
    x_55_phi = true;
    if (!(true)) {
      const uint scalar_offset_2 = ((16u * uint(0))) / 4;
      const int x_51 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
      const int x_53 = asint(x_6[1].x);
      x_54 = (x_51 == x_53);
      x_55_phi = x_54;
    }
    if (!(x_55_phi)) {
      break;
    }
  }
  while (true) {
    if ((x_GLF_global_loop_count < 100)) {
    } else {
      break;
    }
    x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_69 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_70 = float(x_69);
    x_GLF_color = float4(x_70, x_70, x_70, x_70);
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
