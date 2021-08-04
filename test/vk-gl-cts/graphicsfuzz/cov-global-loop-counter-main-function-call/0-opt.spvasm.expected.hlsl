static int x_GLF_global_loop_count = 0;
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int func_() {
  while (true) {
    if ((x_GLF_global_loop_count < 100)) {
    } else {
      break;
    }
    x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_78 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
    return x_78;
  }
  const int x_80 = asint(x_7[2].x);
  return x_80;
}

void main_1() {
  int a = 0;
  x_GLF_global_loop_count = 0;
  while (true) {
    x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
    if (false) {
      return;
    }
    {
      if ((true & (x_GLF_global_loop_count < 100))) {
      } else {
        break;
      }
    }
  }
  const int x_42 = func_();
  a = x_42;
  const int x_43 = a;
  const int x_45 = asint(x_7[2].x);
  if ((x_43 == x_45)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_51 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_54 = asint(x_7[1].x);
    const int x_57 = asint(x_7[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_60 = asint(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_GLF_color = float4(float(x_51), float(x_54), float(x_57), float(x_60));
  } else {
    const int x_64 = asint(x_7[1].x);
    const float x_65 = float(x_64);
    x_GLF_color = float4(x_65, x_65, x_65, x_65);
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
