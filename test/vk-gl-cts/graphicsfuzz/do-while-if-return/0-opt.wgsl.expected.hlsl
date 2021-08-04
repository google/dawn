cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int func_() {
  int loop_count = 0;
  int x_38_phi = 0;
  loop_count = 0;
  x_38_phi = 0;
  while (true) {
    int x_39 = 0;
    int x_45_phi = 0;
    const int x_43 = (x_38_phi + 1);
    loop_count = x_43;
    x_45_phi = x_43;
    while (true) {
      x_39 = (x_45_phi + 1);
      loop_count = x_39;
      const float x_50 = asfloat(x_7[0].x);
      const float x_52 = asfloat(x_7[0].y);
      if ((x_50 < x_52)) {
        return 1;
      }
      const float x_57 = asfloat(x_7[0].x);
      const float x_59 = asfloat(x_7[0].y);
      if ((x_57 < x_59)) {
        break;
      }
      {
        x_45_phi = x_39;
        if ((x_39 < 100)) {
        } else {
          break;
        }
      }
    }
    {
      x_38_phi = x_39;
      if ((x_39 < 100)) {
      } else {
        break;
      }
    }
  }
  return 0;
}

void main_1() {
  const int x_31 = func_();
  if ((x_31 == 1)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
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
