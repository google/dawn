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
  float v_2[4] = v_1.clipDistance;
  float v_3[4] = v_1.clipDistance;
  float v_4[4] = v_1.clipDistance;
  float v_5[4] = v_1.clipDistance;
  main_outputs v_6 = {v_1.position, float4(v_2[0u], v_3[1u], v_4[2u], v_5[3u])};
  return v_6;
}

