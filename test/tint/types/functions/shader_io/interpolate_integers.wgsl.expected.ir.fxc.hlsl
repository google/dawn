struct Interface {
  int i;
  uint u;
  int4 vi;
  uint4 vu;
  float4 pos;
};

struct vert_main_outputs {
  nointerpolation int Interface_i : TEXCOORD0;
  nointerpolation uint Interface_u : TEXCOORD1;
  nointerpolation int4 Interface_vi : TEXCOORD2;
  nointerpolation uint4 Interface_vu : TEXCOORD3;
  float4 Interface_pos : SV_Position;
};

struct frag_main_outputs {
  int tint_symbol : SV_Target0;
};

struct frag_main_inputs {
  nointerpolation int Interface_i : TEXCOORD0;
  nointerpolation uint Interface_u : TEXCOORD1;
  nointerpolation int4 Interface_vi : TEXCOORD2;
  nointerpolation uint4 Interface_vu : TEXCOORD3;
  float4 Interface_pos : SV_Position;
};


Interface vert_main_inner() {
  Interface v = (Interface)0;
  return v;
}

int frag_main_inner(Interface inputs) {
  return inputs.i;
}

vert_main_outputs vert_main() {
  Interface v_1 = vert_main_inner();
  Interface v_2 = v_1;
  Interface v_3 = v_1;
  Interface v_4 = v_1;
  Interface v_5 = v_1;
  Interface v_6 = v_1;
  vert_main_outputs v_7 = {v_2.i, v_3.u, v_4.vi, v_5.vu, v_6.pos};
  return v_7;
}

frag_main_outputs frag_main(frag_main_inputs inputs) {
  Interface v_8 = {inputs.Interface_i, inputs.Interface_u, inputs.Interface_vi, inputs.Interface_vu, float4(inputs.Interface_pos.xyz, (1.0f / inputs.Interface_pos[3u]))};
  frag_main_outputs v_9 = {frag_main_inner(v_8)};
  return v_9;
}

