struct VertexOutputs {
  float4 position;
  float clipDistance[6];
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float4 VertexOutputs_clipDistance0 : SV_ClipDistance0;
  float2 VertexOutputs_clipDistance1 : SV_ClipDistance1;
};


VertexOutputs main_inner() {
  VertexOutputs v = {float4(1.0f, 2.0f, 3.0f, 4.0f), (float[6])0};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  VertexOutputs v_3 = v_1;
  float v_4[6] = v_3.clipDistance;
  float v_5[6] = v_3.clipDistance;
  float v_6[6] = v_3.clipDistance;
  float v_7[6] = v_3.clipDistance;
  float4 v_8 = float4(v_4[0u], v_5[1u], v_6[2u], v_7[3u]);
  float v_9[6] = v_3.clipDistance;
  float v_10[6] = v_3.clipDistance;
  main_outputs v_11 = {v_2.position, v_8, float2(v_9[4u], v_10[5u])};
  return v_11;
}

