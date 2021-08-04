static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[4];
};
cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[1];
};

void main_1() {
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_28 = asint(x_5[scalar_offset / 4][scalar_offset % 4]);
  const int x_31 = asint(x_5[1].x);
  const int x_34 = asint(x_5[1].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_37 = asint(x_5[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  x_GLF_color = float4(float(x_28), float(x_31), float(x_34), float(x_37));
  while (true) {
    const int x_45 = asint(x_7[0].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_47 = asint(x_5[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if ((x_45 == x_47)) {
      {
        if (false) {
        } else {
          break;
        }
      }
      continue;
    }
    const int x_52 = asint(x_7[0].x);
    const int x_54 = asint(x_5[2].x);
    if ((x_52 == x_54)) {
      break;
    }
    const int x_59 = asint(x_7[0].x);
    const int x_61 = asint(x_5[3].x);
    if ((x_59 == x_61)) {
      discard;
    }
    return;
    {
      if (false) {
      } else {
        break;
      }
    }
  }
  const int x_66 = asint(x_5[1].x);
  const float x_67 = float(x_66);
  x_GLF_color = float4(x_67, x_67, x_67, x_67);
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
