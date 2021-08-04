void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[2];
};
cbuffer cbuffer_x_11 : register(b1, space0) {
  uint4 x_11[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int i = 0;
  int j = 0;
  int k = 0;
  color = float4(1.0f, 1.0f, 1.0f, 1.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_37 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
  i = x_37;
  while (true) {
    const int x_42 = i;
    const int x_44 = asint(x_7[1].x);
    if ((x_42 < x_44)) {
    } else {
      break;
    }
    switch(i) {
      case 2: {
        const int x_83 = i;
        const uint scalar_offset_1 = ((16u * uint(0))) / 4;
        const float x_85 = asfloat(x_11[scalar_offset_1 / 4][scalar_offset_1 % 4]);
        set_float4(color, x_83, x_85);
        break;
      }
      case 1: {
        const uint scalar_offset_2 = ((16u * uint(0))) / 4;
        const int x_52 = asint(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
        j = x_52;
        {
          for(; (i > i); j = (j + 1)) {
            const uint scalar_offset_3 = ((16u * uint(0))) / 4;
            const int x_62 = asint(x_7[scalar_offset_3 / 4][scalar_offset_3 % 4]);
            k = x_62;
            {
              for(; (k < i); k = (k + 1)) {
                const int x_71 = k;
                const uint scalar_offset_4 = ((16u * uint(0))) / 4;
                const float x_73 = asfloat(x_11[scalar_offset_4 / 4][scalar_offset_4 % 4]);
                set_float4(color, x_71, x_73);
              }
            }
          }
        }
        const int x_79 = i;
        const uint scalar_offset_5 = ((16u * uint(0))) / 4;
        const float x_81 = asfloat(x_11[scalar_offset_5 / 4][scalar_offset_5 % 4]);
        set_float4(color, x_79, x_81);
        break;
      }
      default: {
        break;
      }
    }
    {
      i = (i + 1);
    }
  }
  x_GLF_color = color;
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
