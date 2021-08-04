cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  uint a = 0u;
  uint b = 0u;
  uint c = 0u;
  uint d = 0u;
  uint e = 0u;
  uint f = 0u;
  const uint x_41 = x_6[0].x;
  a = ((77u + x_41) >> 32u);
  const uint x_45 = x_6[0].x;
  b = ((3243u + x_45) >> 33u);
  const uint x_49 = x_6[0].x;
  c = ((23u + x_49) >> 345u);
  const uint x_53 = x_6[0].x;
  d = ((2395u + x_53) << 32u);
  const uint x_57 = x_6[0].x;
  e = ((290485u + x_57) << 33u);
  const uint x_61 = x_6[0].x;
  f = ((44321u + x_61) << 345u);
  if ((a != 1u)) {
    a = 1u;
  }
  if ((b != 0u)) {
    b = 0u;
  }
  if ((c != 1u)) {
    c = 1u;
  }
  if ((d != 0u)) {
    d = 0u;
  }
  if ((e != 1u)) {
    e = 1u;
  }
  if ((f != 0u)) {
    f = 0u;
  }
  if (((((((a == 1u) & (b == 0u)) & (c == 1u)) & (d == 0u)) & (e == 1u)) & (f == 0u))) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
