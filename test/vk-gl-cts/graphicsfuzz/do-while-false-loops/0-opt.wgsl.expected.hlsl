static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 f_() {
  int iteration = 0;
  int k = 0;
  iteration = 0;
  k = 0;
  {
    for(; (k < 100); k = (k + 1)) {
      iteration = (iteration + 1);
    }
  }
  if ((iteration < 100)) {
    return float3(1.0f, float((iteration - 1)), float((iteration - 1)));
  } else {
    while (true) {
      while (true) {
        return float3(1.0f, 0.0f, 0.0f);
      }
      return float3(0.0f, 0.0f, 0.0f);
    }
    return float3(0.0f, 0.0f, 0.0f);
  }
  return float3(0.0f, 0.0f, 0.0f);
}

void main_1() {
  const float3 x_35 = f_();
  x_GLF_color = float4(x_35.x, x_35.y, x_35.z, 1.0f);
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
