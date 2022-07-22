static float4 out_var_SV_TARGET = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  out_var_SV_TARGET = float4(asfloat(0x7fc00000u), asfloat(0x7fc00000u), asfloat(0x7fc00000u), asfloat(0x7fc00000u));
  return;
}

struct main_out {
  float4 out_var_SV_TARGET_1;
};
struct tint_symbol {
  float4 out_var_SV_TARGET_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {out_var_SV_TARGET};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.out_var_SV_TARGET_1 = inner_result.out_var_SV_TARGET_1;
  return wrapper_result;
}
