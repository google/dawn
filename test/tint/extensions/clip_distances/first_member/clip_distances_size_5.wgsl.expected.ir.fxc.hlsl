struct VertexOutputs {
  float clipDistance[5];
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float4 VertexOutputs_clipDistance0 : SV_ClipDistance0;
  float VertexOutputs_clipDistance1 : SV_ClipDistance1;
};


VertexOutputs main_inner() {
  VertexOutputs v = {(float[5])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  float v_3[5] = v_2.clipDistance;
  float v_4[5] = v_2.clipDistance;
  float v_5[5] = v_2.clipDistance;
  float v_6[5] = v_2.clipDistance;
  float v_7[5] = v_2.clipDistance;
  VertexOutputs v_8 = v_1;
  main_outputs v_9 = {v_8.position, float4(v_3[0u], v_4[1u], v_5[2u], v_6[3u]), v_7[4u]};
  return v_9;
}

