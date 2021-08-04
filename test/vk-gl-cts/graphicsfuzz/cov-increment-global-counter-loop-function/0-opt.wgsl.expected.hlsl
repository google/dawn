static int x_GLF_global_loop_count = 0;
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void func_() {
  int x_66_phi = 0;
  const int x_62 = asint(x_7[1].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_64 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
  x_66_phi = x_64;
  while (true) {
    int x_67 = 0;
    const int x_66 = x_66_phi;
    const int x_70 = asint(x_7[3].x);
    if ((x_66 < x_70)) {
    } else {
      break;
    }
    {
      x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
      x_67 = (x_66 + 1);
      x_66_phi = x_67;
    }
  }
  if ((x_62 < x_62)) {
    return;
  }
  return;
}

void main_1() {
  x_GLF_global_loop_count = 0;
  while (true) {
    if ((x_GLF_global_loop_count < 10)) {
    } else {
      break;
    }
    {
      x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
      func_();
    }
  }
  {
    for(; (x_GLF_global_loop_count < 10); x_GLF_global_loop_count = (x_GLF_global_loop_count + 1)) {
    }
  }
  const int x_42 = x_GLF_global_loop_count;
  const int x_44 = asint(x_7[2].x);
  if ((x_42 == x_44)) {
    const int x_50 = asint(x_7[1].x);
    const float x_51 = float(x_50);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_53 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_54 = float(x_53);
    x_GLF_color = float4(x_51, x_54, x_54, x_51);
  } else {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_57 = asint(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const float x_58 = float(x_57);
    x_GLF_color = float4(x_58, x_58, x_58, x_58);
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
