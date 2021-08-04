static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  bool c1 = false;
  float2 uv = float2(0.0f, 0.0f);
  int i = 0;
  bool x_37 = false;
  bool x_37_phi = false;
  int x_9_phi = 0;
  const float x_34 = uv.y;
  const bool x_35 = (x_34 < 0.25f);
  c1 = x_35;
  i = 0;
  x_37_phi = x_35;
  x_9_phi = 0;
  while (true) {
    x_37 = x_37_phi;
    if ((x_9_phi < 1)) {
    } else {
      break;
    }
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return;
    {
      i = (i + 1);
      x_37_phi = false;
      x_9_phi = 0;
    }
  }
  if (x_37) {
    return;
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
