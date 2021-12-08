SKIP: FAILED

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b1, space0) {
  uint4 x_5[2];
};
cbuffer cbuffer_x_7 : register(b2, space0) {
  uint4 x_7[1];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

void main_1() {
  int i = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_38 = asfloat(x_5[scalar_offset / 4][scalar_offset % 4]);
  x_GLF_color = float4(x_38, x_38, x_38, x_38);
  const float x_41 = asfloat(x_7[0].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_43 = asfloat(x_5[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_41 > x_43)) {
    [loop] while (true) {
      const float x_53 = asfloat(x_5[1].x);
      x_GLF_color = float4(x_53, x_53, x_53, x_53);
      {
        if (true) {
        } else {
          break;
        }
      }
    }
  } else {
    [loop] while (true) {
      [loop] while (true) {
        if (true) {
        } else {
          break;
        }
        const int x_13 = asint(x_10[1].x);
        i = x_13;
        [loop] while (true) {
          const int x_14 = i;
          const uint scalar_offset_2 = ((16u * uint(0))) / 4;
          const int x_15 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
          if ((x_14 < x_15)) {
          } else {
            break;
          }
          const float x_73 = asfloat(x_5[1].x);
          const uint scalar_offset_3 = ((16u * uint(0))) / 4;
          const float x_75 = asfloat(x_5[scalar_offset_3 / 4][scalar_offset_3 % 4]);
          const uint scalar_offset_4 = ((16u * uint(0))) / 4;
          const float x_77 = asfloat(x_5[scalar_offset_4 / 4][scalar_offset_4 % 4]);
          const float x_79 = asfloat(x_5[1].x);
          x_GLF_color = float4(x_73, x_75, x_77, x_79);
          {
            i = (i + 1);
          }
        }
        break;
      }
      {
        const float x_82 = asfloat(x_7[0].x);
        const uint scalar_offset_5 = ((16u * uint(0))) / 4;
        const float x_84 = asfloat(x_5[scalar_offset_5 / 4][scalar_offset_5 % 4]);
        if ((x_82 > x_84)) {
        } else {
          break;
        }
      }
    }
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
C:\src\tint\test\Shader@0x000002497A3112E0(21,19-22): error X3696: infinite loop detected - loop never exits

