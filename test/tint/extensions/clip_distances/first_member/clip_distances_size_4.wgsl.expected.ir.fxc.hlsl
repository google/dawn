struct VertexOutputs {
  float clipDistance[4];
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float4 VertexOutputs_clipDistance0 : SV_ClipDistance0;
};


VertexOutputs main_inner() {
  VertexOutputs v = {(float[4])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  float v_3[4] = v_2.clipDistance;
  float v_4[4] = v_2.clipDistance;
  float v_5[4] = v_2.clipDistance;
  float v_6[4] = v_2.clipDistance;
  VertexOutputs v_7 = v_1;
  main_outputs v_8 = {v_7.position, float4(v_3[0u], v_4[1u], v_5[2u], v_6[3u])};
  return v_8;
}

