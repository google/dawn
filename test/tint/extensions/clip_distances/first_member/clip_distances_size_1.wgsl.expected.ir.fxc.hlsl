struct VertexOutputs {
  float clipDistance[1];
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float VertexOutputs_clipDistance0 : SV_ClipDistance0;
};


VertexOutputs main_inner() {
  VertexOutputs v = {(float[1])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  float v_3[1] = v_2.clipDistance;
  VertexOutputs v_4 = v_1;
  main_outputs v_5 = {v_4.position, v_3[0u]};
  return v_5;
}

