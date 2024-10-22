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
  VertexOutputs v_2 = v_1;
  float v_3[6] = v_2.clipDistance;
  float v_4[6] = v_2.clipDistance;
  float v_5[6] = v_2.clipDistance;
  float v_6[6] = v_2.clipDistance;
  float4 v_7 = float4(v_3[0u], v_4[1u], v_5[2u], v_6[3u]);
  float v_8[6] = v_2.clipDistance;
  float v_9[6] = v_2.clipDistance;
  VertexOutputs v_10 = v_1;
  main_outputs v_11 = {v_10.position, v_7, float2(v_8[4u], v_9[5u])};
  return v_11;
}

