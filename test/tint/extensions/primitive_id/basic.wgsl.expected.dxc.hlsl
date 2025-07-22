struct main_outputs {
  float4 tint_symbol : SV_Target0;
};

struct main_inputs {
  uint prim_id : SV_PrimitiveId;
};


float4 main_inner(uint prim_id) {
  return float4((float(prim_id)).xxxx);
}

main_outputs main(main_inputs inputs) {
  main_outputs v = {main_inner(inputs.prim_id)};
  return v;
}

