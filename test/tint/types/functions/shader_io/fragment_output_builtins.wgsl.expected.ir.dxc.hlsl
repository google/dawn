struct main1_outputs {
  float tint_symbol : SV_Depth;
};

struct main2_outputs {
  uint tint_symbol_1 : SV_Coverage;
};


float main1_inner() {
  return 1.0f;
}

uint main2_inner() {
  return 1u;
}

main1_outputs main1() {
  main1_outputs v = {main1_inner()};
  return v;
}

main2_outputs main2() {
  main2_outputs v_1 = {main2_inner()};
  return v_1;
}

