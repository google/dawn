struct VertexOutputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
  float4 position;
};

struct main_outputs {
  nointerpolation int VertexOutputs_loc0 : TEXCOORD0;
  nointerpolation uint VertexOutputs_loc1 : TEXCOORD1;
  float VertexOutputs_loc2 : TEXCOORD2;
  float4 VertexOutputs_loc3 : TEXCOORD3;
  float4 VertexOutputs_position : SV_Position;
};


VertexOutputs main_inner() {
  VertexOutputs v = {int(1), 1u, 1.0f, float4(1.0f, 2.0f, 3.0f, 4.0f), (0.0f).xxxx};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  VertexOutputs v_3 = v_1;
  VertexOutputs v_4 = v_1;
  VertexOutputs v_5 = v_1;
  VertexOutputs v_6 = v_1;
  main_outputs v_7 = {v_2.loc0, v_3.loc1, v_4.loc2, v_5.loc3, v_6.position};
  return v_7;
}

