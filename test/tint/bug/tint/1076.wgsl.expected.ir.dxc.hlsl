struct FragIn {
  float a;
  uint mask;
};

struct main_outputs {
  float FragIn_a : SV_Target0;
  uint FragIn_mask : SV_Coverage;
};

struct main_inputs {
  float FragIn_a : TEXCOORD0;
  float b : TEXCOORD1;
  uint FragIn_mask : SV_Coverage;
};


FragIn main_inner(FragIn tint_symbol, float b) {
  if ((tint_symbol.mask == 0u)) {
    return tint_symbol;
  }
  FragIn v = {b, 1u};
  return v;
}

main_outputs main(main_inputs inputs) {
  FragIn v_1 = {inputs.FragIn_a, inputs.FragIn_mask};
  FragIn v_2 = main_inner(v_1, inputs.b);
  FragIn v_3 = v_2;
  FragIn v_4 = v_2;
  main_outputs v_5 = {v_3.a, v_4.mask};
  return v_5;
}

