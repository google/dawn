struct VertexOutputs {
  float clipDistance[7];
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float4 VertexOutputs_clipDistance0 : SV_ClipDistance0;
  float3 VertexOutputs_clipDistance1 : SV_ClipDistance1;
};


VertexOutputs main_inner() {
  VertexOutputs v = {(float[7])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  float v_3[7] = v_2.clipDistance;
  float v_4[7] = v_2.clipDistance;
  float v_5[7] = v_2.clipDistance;
  float v_6[7] = v_2.clipDistance;
  float4 v_7 = float4(v_3[0u], v_4[1u], v_5[2u], v_6[3u]);
  float v_8[7] = v_2.clipDistance;
  float v_9[7] = v_2.clipDistance;
  float v_10[7] = v_2.clipDistance;
  VertexOutputs v_11 = v_1;
  main_outputs v_12 = {v_11.position, v_7, float3(v_8[4u], v_9[5u], v_10[6u])};
  return v_12;
}

