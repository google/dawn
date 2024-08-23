SKIP: FAILED

struct f_outputs {
  float4 tint_symbol : SV_Target0;
};

struct f_inputs {
  float4 fbf;
};


float4 f_inner(float4 fbf) {
  return fbf;
}

f_outputs f(f_inputs inputs) {
  f_outputs v = {f_inner(inputs.fbf)};
  return v;
}

FXC validation failure:
<scrubbed_path>(14,22-27): error X3502: 'f': input parameter 'inputs' missing semantics


tint executable returned error: exit status 1
