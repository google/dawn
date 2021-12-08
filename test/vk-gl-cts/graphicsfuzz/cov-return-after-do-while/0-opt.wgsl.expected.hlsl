SKIP: FAILED

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[2];
};

void main_1() {
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_22 = asint(x_5[scalar_offset / 4][scalar_offset % 4]);
  const int x_25 = asint(x_5[1].x);
  const int x_28 = asint(x_5[1].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_31 = asint(x_5[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  x_GLF_color = float4(float(x_22), float(x_25), float(x_28), float(x_31));
  const int x_35 = asint(x_5[1].x);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_37 = asint(x_5[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  if ((x_35 > x_37)) {
    [loop] while (true) {
      const uint scalar_offset_3 = ((16u * uint(0))) / 4;
      const int x_46 = asint(x_5[scalar_offset_3 / 4][scalar_offset_3 % 4]);
      const float x_47 = float(x_46);
      x_GLF_color = float4(x_47, x_47, x_47, x_47);
      {
        const int x_50 = asint(x_5[1].x);
        const uint scalar_offset_4 = ((16u * uint(0))) / 4;
        const int x_52 = asint(x_5[scalar_offset_4 / 4][scalar_offset_4 % 4]);
        if ((x_50 > x_52)) {
        } else {
          break;
        }
      }
    }
    return;
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
C:\src\tint\test\Shader@0x000001AB97856630(18,19-22): error X3696: infinite loop detected - loop never exits

