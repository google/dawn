struct tint_symbol {
  float value : SV_Depth;
};

float main1_inner() {
  return 1.0f;
}

tint_symbol main1() {
  float inner_result = main1_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

struct tint_symbol_1 {
  uint value : SV_Coverage;
};

uint main2_inner() {
  return 1u;
}

tint_symbol_1 main2() {
  uint inner_result_1 = main2_inner();
  tint_symbol_1 wrapper_result_1 = (tint_symbol_1)0;
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
