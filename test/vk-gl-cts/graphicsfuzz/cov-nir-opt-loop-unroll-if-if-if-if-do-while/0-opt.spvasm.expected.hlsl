static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[2];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[2];
};

void main_1() {
  float a = 0.0f;
  const float x_31 = asfloat(x_5[1].x);
  x_GLF_color = float4(x_31, x_31, x_31, x_31);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_34 = asfloat(x_5[scalar_offset / 4][scalar_offset % 4]);
  a = x_34;
  while (true) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_40 = asfloat(x_5[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_43 = asfloat(x_5[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if (((x_40 / 0.200000003f) < x_43)) {
      return;
    }
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const float x_48 = asfloat(x_5[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_51 = asfloat(x_5[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    if (((x_48 / 0.200000003f) < x_51)) {
      return;
    }
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_56 = asfloat(x_5[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const float x_59 = asfloat(x_5[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    if (((x_56 / 0.200000003f) < x_59)) {
      return;
    }
    const uint scalar_offset_7 = ((16u * uint(0))) / 4;
    const float x_64 = asfloat(x_5[scalar_offset_7 / 4][scalar_offset_7 % 4]);
    const uint scalar_offset_8 = ((16u * uint(0))) / 4;
    const float x_67 = asfloat(x_5[scalar_offset_8 / 4][scalar_offset_8 % 4]);
    if (((x_64 / 0.200000003f) < x_67)) {
      return;
    } else {
      a = 0.0f;
    }
    {
      if (!((a == 0.0f))) {
      } else {
        break;
      }
    }
  }
  const int x_75 = asint(x_8[1].x);
  const uint scalar_offset_9 = ((16u * uint(0))) / 4;
  const int x_78 = asint(x_8[scalar_offset_9 / 4][scalar_offset_9 % 4]);
  const uint scalar_offset_10 = ((16u * uint(0))) / 4;
  const int x_81 = asint(x_8[scalar_offset_10 / 4][scalar_offset_10 % 4]);
  const int x_84 = asint(x_8[1].x);
  x_GLF_color = float4(float(x_75), float(x_78), float(x_81), float(x_84));
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
