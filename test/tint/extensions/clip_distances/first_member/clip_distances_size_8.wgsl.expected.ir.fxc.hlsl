struct VertexOutputs {
  float clipDistance[8];
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float4 VertexOutputs_clipDistance0 : SV_ClipDistance0;
  float4 VertexOutputs_clipDistance1 : SV_ClipDistance1;
};


VertexOutputs main_inner() {
  VertexOutputs v = {(float[8])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  float v_3[8] = v_2.clipDistance;
  float v_4[8] = v_2.clipDistance;
  float v_5[8] = v_2.clipDistance;
  float v_6[8] = v_2.clipDistance;
  float4 v_7 = float4(v_3[0u], v_4[1u], v_5[2u], v_6[3u]);
  float v_8[8] = v_2.clipDistance;
  float v_9[8] = v_2.clipDistance;
  float v_10[8] = v_2.clipDistance;
  float v_11[8] = v_2.clipDistance;
  VertexOutputs v_12 = v_1;
  main_outputs v_13 = {v_12.position, v_7, float4(v_8[4u], v_9[5u], v_10[6u], v_11[7u])};
  return v_13;
}

