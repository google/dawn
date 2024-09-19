struct main0_outputs {
  int tint_symbol : SV_Target0;
};

struct main1_outputs {
  uint tint_symbol_1 : SV_Target1;
};

struct main2_outputs {
  float tint_symbol_2 : SV_Target2;
};

struct main3_outputs {
  float4 tint_symbol_3 : SV_Target3;
};


int main0_inner() {
  return int(1);
}

uint main1_inner() {
  return 1u;
}

float main2_inner() {
  return 1.0f;
}

float4 main3_inner() {
  return float4(1.0f, 2.0f, 3.0f, 4.0f);
}

main0_outputs main0() {
  main0_outputs v = {main0_inner()};
  return v;
}

main1_outputs main1() {
  main1_outputs v_1 = {main1_inner()};
  return v_1;
}

main2_outputs main2() {
  main2_outputs v_2 = {main2_inner()};
  return v_2;
}

main3_outputs main3() {
  main3_outputs v_3 = {main3_inner()};
  return v_3;
}

