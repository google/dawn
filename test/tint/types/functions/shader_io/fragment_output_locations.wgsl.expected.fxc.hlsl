struct tint_symbol {
  int value : SV_Target0;
};

int main0_inner() {
  return 1;
}

tint_symbol main0() {
  const int inner_result = main0_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

struct tint_symbol_1 {
  uint value : SV_Target1;
};

uint main1_inner() {
  return 1u;
}

tint_symbol_1 main1() {
  const uint inner_result_1 = main1_inner();
  tint_symbol_1 wrapper_result_1 = (tint_symbol_1)0;
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}

struct tint_symbol_2 {
  float value : SV_Target2;
};

float main2_inner() {
  return 1.0f;
}

tint_symbol_2 main2() {
  const float inner_result_2 = main2_inner();
  tint_symbol_2 wrapper_result_2 = (tint_symbol_2)0;
  wrapper_result_2.value = inner_result_2;
  return wrapper_result_2;
}

struct tint_symbol_3 {
  float4 value : SV_Target3;
};

float4 main3_inner() {
  return float4(1.0f, 2.0f, 3.0f, 4.0f);
}

tint_symbol_3 main3() {
  const float4 inner_result_3 = main3_inner();
  tint_symbol_3 wrapper_result_3 = (tint_symbol_3)0;
  wrapper_result_3.value = inner_result_3;
  return wrapper_result_3;
}
