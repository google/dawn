struct Interface {
  float col1;
  float col2;
  float4 pos;
};

struct vert_main_outputs {
  float Interface_col1 : TEXCOORD1;
  float Interface_col2 : TEXCOORD2;
  float4 Interface_pos : SV_Position;
};

struct frag_main_inputs {
  float Interface_col1 : TEXCOORD1;
  float Interface_col2 : TEXCOORD2;
  float4 Interface_pos : SV_Position;
};


Interface vert_main_inner() {
  Interface v = {0.40000000596046447754f, 0.60000002384185791016f, (0.0f).xxxx};
  return v;
}

void frag_main_inner(Interface colors) {
  float r = colors.col1;
  float g = colors.col2;
}

vert_main_outputs vert_main() {
  Interface v_1 = vert_main_inner();
  vert_main_outputs v_2 = {v_1.col1, v_1.col2, v_1.pos};
  return v_2;
}

void frag_main(frag_main_inputs inputs) {
  Interface v_3 = {inputs.Interface_col1, inputs.Interface_col2, float4(inputs.Interface_pos.xyz, (1.0f / inputs.Interface_pos[3u]))};
  frag_main_inner(v_3);
}

