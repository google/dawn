struct main_outputs {
  float4 tint_symbol : SV_Target0;
};

struct main_inputs {
  float3 bary_coord : SV_Barycentrics;
};


float4 main_inner(float3 bary_coord) {
  return float4(bary_coord, 1.0f);
}

main_outputs main(main_inputs inputs) {
  main_outputs v = {main_inner(inputs.bary_coord)};
  return v;
}

