struct VertexOutputs {
  float4 position;
  float clipDistance[4];
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float4 VertexOutputs_clipDistance0 : SV_ClipDistance0;
};


VertexOutputs main_inner() {
  VertexOutputs v = {float4(1.0f, 2.0f, 3.0f, 4.0f), (float[4])0};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  VertexOutputs v_3 = v_1;
  float v_4[4] = v_3.clipDistance;
  float v_5[4] = v_3.clipDistance;
  float v_6[4] = v_3.clipDistance;
  float v_7[4] = v_3.clipDistance;
  main_outputs v_8 = {v_2.position, float4(v_4[0u], v_5[1u], v_6[2u], v_7[3u])};
  return v_8;
}

