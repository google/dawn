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
  float v_2[7] = v_1.clipDistance;
  float v_3[7] = v_1.clipDistance;
  float v_4[7] = v_1.clipDistance;
  float v_5[7] = v_1.clipDistance;
  float4 v_6 = float4(v_2[0u], v_3[1u], v_4[2u], v_5[3u]);
  float v_7[7] = v_1.clipDistance;
  float v_8[7] = v_1.clipDistance;
  float v_9[7] = v_1.clipDistance;
  main_outputs v_10 = {v_1.position, v_6, float3(v_7[4u], v_8[5u], v_9[6u])};
  return v_10;
}

