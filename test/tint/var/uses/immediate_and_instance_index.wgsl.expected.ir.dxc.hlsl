struct main_outputs {
  float4 tint_symbol : SV_Position;
};

struct main_inputs {
  uint b : SV_InstanceID;
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[1];
};
float4 main_inner(uint b) {
  float v = asfloat(a[0u].x);
  return float4(((v + float(b))).xxxx);
}

main_outputs main(main_inputs inputs) {
  main_outputs v_1 = {main_inner(inputs.b)};
  return v_1;
}

