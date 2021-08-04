static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 mand_() {
  bool x_40 = false;
  float3 x_41 = float3(0.0f, 0.0f, 0.0f);
  int k = 0;
  while (true) {
    k = 0;
    while (true) {
      if ((k < 1000)) {
      } else {
        break;
      }
      x_40 = true;
      x_41 = float3(1.0f, 1.0f, 1.0f);
      break;
    }
    if (x_40) {
      break;
    }
    discard;
  }
  return x_41;
}

void main_1() {
  int i = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  i = 0;
  while (true) {
    if ((i < 4)) {
    } else {
      break;
    }
    {
      const float3 x_38 = mand_();
      i = (i + 1);
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
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
