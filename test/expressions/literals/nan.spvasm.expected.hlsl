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

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {out_var_SV_TARGET};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.out_var_SV_TARGET_1};
  return tint_symbol_2;
}
