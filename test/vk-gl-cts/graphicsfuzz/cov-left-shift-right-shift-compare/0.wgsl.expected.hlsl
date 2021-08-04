static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[3];
};

void main_1() {
  int x_32_phi = 0;
  const int x_24 = asint(x_5[1].x);
  const float x_25 = float(x_24);
  x_GLF_color = float4(x_25, x_25, x_25, x_25);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_28 = asint(x_5[scalar_offset / 4][scalar_offset % 4]);
  const int x_30 = ((x_28 << asuint(x_28)) >> asuint(1));
  x_32_phi = x_24;
  while (true) {
    const int x_32 = x_32_phi;
    if ((x_30 < 10)) {
    } else {
      break;
    }
    int x_33 = 0;
    x_33 = (x_32 + 1);
    const int x_39 = asint(x_5[2].x);
    if ((x_33 == asint(x_39))) {
      const float x_43 = float(x_28);
      x_GLF_color = float4(x_43, x_25, x_25, x_43);
      break;
    }
    {
      x_32_phi = x_33;
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
