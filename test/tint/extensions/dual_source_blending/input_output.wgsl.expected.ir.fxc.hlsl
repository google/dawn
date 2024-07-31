struct FragOutput {
  float4 color;
  float4 blend;
};

struct FragInput {
  float4 a;
  float4 b;
};

struct frag_main_outputs {
  float4 FragOutput_color : SV_Target0;
  float4 FragOutput_blend : SV_Target1;
};

struct frag_main_inputs {
  float4 FragInput_a : TEXCOORD0;
  float4 FragInput_b : TEXCOORD1;
};


FragOutput frag_main_inner(FragInput tint_symbol) {
  FragOutput output = (FragOutput)0;
  output.color = tint_symbol.a;
  output.blend = tint_symbol.b;
  FragOutput v = output;
  return v;
}

frag_main_outputs frag_main(frag_main_inputs inputs) {
  FragInput v_1 = {inputs.FragInput_a, inputs.FragInput_b};
  FragOutput v_2 = frag_main_inner(v_1);
  FragOutput v_3 = v_2;
  FragOutput v_4 = v_2;
  frag_main_outputs v_5 = {v_3.color, v_4.blend};
  return v_5;
}

