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

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_2;
}
