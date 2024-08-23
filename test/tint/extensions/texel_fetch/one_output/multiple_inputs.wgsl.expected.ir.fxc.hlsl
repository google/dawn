SKIP: FAILED

struct f_outputs {
  float4 tint_symbol : SV_Target0;
};

struct f_inputs {
  float4 fbf_1;
  float4 fbf_3;
};


float4 f_inner(float4 fbf_1, float4 fbf_3) {
  return (fbf_1 + fbf_3);
}

f_outputs f(f_inputs inputs) {
  f_outputs v = {f_inner(inputs.fbf_1, inputs.fbf_3)};
  return v;
}

FXC validation failure:
<scrubbed_path>(15,22-27): error X3502: 'f': input parameter 'inputs' missing semantics


tint executable returned error: exit status 1
