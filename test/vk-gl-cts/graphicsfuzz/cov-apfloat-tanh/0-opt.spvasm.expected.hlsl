static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float dist1 = 0.0f;
  float dist2 = 0.0f;
  v = float4(1.0f, 2.0f, 3.0f, 4.0f);
  dist1 = distance(tanh(v), (sinh(v) / cosh(v)));
  dist2 = distance(tanh(v), float4(0.761590004f, 0.964030027f, 0.995050013f, 0.999329984f));
  if (((dist1 < 0.100000001f) & (dist2 < 0.100000001f))) {
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
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
