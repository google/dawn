struct VertexOutputs {
  float clipDistance[6];
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float4 VertexOutputs_clipDistance0 : SV_ClipDistance0;
  float2 VertexOutputs_clipDistance1 : SV_ClipDistance1;
};


VertexOutputs main_inner() {
  VertexOutputs v = {(float[6])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  float v_2[6] = v_1.clipDistance;
  float v_3[6] = v_1.clipDistance;
  float v_4[6] = v_1.clipDistance;
  float v_5[6] = v_1.clipDistance;
  float4 v_6 = float4(v_2[0u], v_3[1u], v_4[2u], v_5[3u]);
  float v_7[6] = v_1.clipDistance;
  float v_8[6] = v_1.clipDistance;
  main_outputs v_9 = {v_1.position, v_6, float2(v_7[4u], v_8[5u])};
  return v_9;
}

