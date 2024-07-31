struct VSOut {
  float4 pos;
};

struct main_outputs {
  float4 VSOut_pos : SV_Position;
};


void foo(inout VSOut tint_symbol) {
  float4 pos = float4(1.0f, 2.0f, 3.0f, 4.0f);
  tint_symbol.pos = pos;
}

VSOut main_inner() {
  VSOut tint_symbol = (VSOut)0;
  foo(tint_symbol);
  VSOut v = tint_symbol;
  return v;
}

main_outputs main() {
  VSOut v_1 = main_inner();
  main_outputs v_2 = {v_1.pos};
  return v_2;
}

