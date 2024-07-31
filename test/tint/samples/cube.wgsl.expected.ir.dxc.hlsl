struct VertexOutput {
  float4 vtxFragColor;
  float4 Position;
};

struct VertexInput {
  float4 cur_position;
  float4 color;
};

struct vtx_main_outputs {
  float4 VertexOutput_vtxFragColor : TEXCOORD0;
  float4 VertexOutput_Position : SV_Position;
};

struct vtx_main_inputs {
  float4 VertexInput_cur_position : TEXCOORD0;
  float4 VertexInput_color : TEXCOORD1;
};

struct frag_main_outputs {
  float4 tint_symbol : SV_Target0;
};

struct frag_main_inputs {
  float4 fragColor : TEXCOORD0;
};


cbuffer cbuffer_uniforms : register(b0) {
  uint4 uniforms[4];
};
float4x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(uniforms[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(uniforms[((16u + start_byte_offset) / 16u)]);
  float4 v_3 = asfloat(uniforms[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_1, v_2, v_3, asfloat(uniforms[((48u + start_byte_offset) / 16u)]));
}

VertexOutput vtx_main_inner(VertexInput input) {
  VertexOutput output = (VertexOutput)0;
  output.Position = mul(input.cur_position, v(0u));
  output.vtxFragColor = input.color;
  VertexOutput v_4 = output;
  return v_4;
}

float4 frag_main_inner(float4 fragColor) {
  return fragColor;
}

vtx_main_outputs vtx_main(vtx_main_inputs inputs) {
  VertexInput v_5 = {inputs.VertexInput_cur_position, inputs.VertexInput_color};
  VertexOutput v_6 = vtx_main_inner(v_5);
  VertexOutput v_7 = v_6;
  VertexOutput v_8 = v_6;
  vtx_main_outputs v_9 = {v_7.vtxFragColor, v_8.Position};
  return v_9;
}

frag_main_outputs frag_main(frag_main_inputs inputs) {
  frag_main_outputs v_10 = {frag_main_inner(inputs.fragColor)};
  return v_10;
}

