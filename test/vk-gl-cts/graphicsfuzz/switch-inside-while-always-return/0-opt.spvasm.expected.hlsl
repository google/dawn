cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int merge_() {
  const float x_40 = asfloat(x_6[0].x);
  if ((x_40 < 0.0f)) {
    return 0;
  }
  return 0;
}

void main_1() {
  int res = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  while (true) {
    const float x_32 = asfloat(x_6[0].x);
    switch(int(x_32)) {
      case 0: {
        return;
        break;
      }
      default: {
        break;
      }
    }
    {
      if (false) {
      } else {
        break;
      }
    }
  }
  const int x_8 = merge_();
  res = x_8;
  const float x_36 = float(res);
  x_GLF_color = float4(x_36, x_36, x_36, x_36);
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
