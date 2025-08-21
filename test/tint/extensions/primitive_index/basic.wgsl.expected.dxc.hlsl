struct main_outputs {
  float4 tint_symbol : SV_Target0;
};

struct main_inputs {
  uint prim_idx : SV_PrimitiveId;
};


float4 main_inner(uint prim_idx) {
  return float4((float(prim_idx)).xxxx);
}

main_outputs main(main_inputs inputs) {
  main_outputs v = {main_inner(inputs.prim_idx)};
  return v;
}

