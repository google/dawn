SKIP: FAILED

cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float A[2] = (float[2])0;
  int i = 0;
  int j = 0;
  bool x_101 = false;
  bool x_102_phi = false;
  const float x_39 = asfloat(x_6[1].x);
  A[0] = x_39;
  const float x_42 = asfloat(x_6[1].x);
  A[1] = x_42;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_45 = asint(x_9[scalar_offset / 4][scalar_offset % 4]);
  i = x_45;
  [loop] while (true) {
    const int x_50 = i;
    const int x_52 = asint(x_9[3].x);
    if ((x_50 < x_52)) {
    } else {
      break;
    }
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_56 = asint(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    j = x_56;
    [loop] while (true) {
      const int x_61 = j;
      const int x_63 = asint(x_9[2].x);
      if ((x_61 < x_63)) {
      } else {
        break;
      }
      switch(j) {
        case 1: {
          const int x_78 = i;
          const uint scalar_offset_2 = ((16u * uint(0))) / 4;
          const float x_80 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
          A[x_78] = x_80;
          break;
        }
        case 0: {
          if ((-2147483648 < i)) {
            {
              j = (j + 1);
            }
            continue;
          }
          const int x_74 = i;
          const float x_76 = asfloat(x_6[2].x);
          A[x_74] = x_76;
          break;
        }
        default: {
          break;
        }
      }
      {
        j = (j + 1);
      }
    }
    {
      i = (i + 1);
    }
  }
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_87 = asint(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const float x_89 = A[x_87];
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const float x_91 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const bool x_92 = (x_89 == x_91);
  x_102_phi = x_92;
  if (x_92) {
    const int x_96 = asint(x_9[1].x);
    const float x_98 = A[x_96];
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_100 = asfloat(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    x_101 = (x_98 == x_100);
    x_102_phi = x_101;
  }
  if (x_102_phi) {
    const int x_107 = asint(x_9[1].x);
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_110 = asint(x_9[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const uint scalar_offset_7 = ((16u * uint(0))) / 4;
    const int x_113 = asint(x_9[scalar_offset_7 / 4][scalar_offset_7 % 4]);
    const int x_116 = asint(x_9[1].x);
    x_GLF_color = float4(float(x_107), float(x_110), float(x_113), float(x_116));
  } else {
    const int x_120 = asint(x_9[1].x);
    const float x_121 = float(x_120);
    x_GLF_color = float4(x_121, x_121, x_121, x_121);
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
C:\src\tint\test\Shader@0x0000027536C22E40(52,13-21): error X3708: continue cannot be used in a switch

