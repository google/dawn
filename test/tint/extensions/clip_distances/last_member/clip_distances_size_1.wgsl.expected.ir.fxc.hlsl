struct VertexOutputs {
  float4 position;
  float clipDistance[1];
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float VertexOutputs_clipDistance0 : SV_ClipDistance0;
};


VertexOutputs main_inner() {
  VertexOutputs v = {float4(1.0f, 2.0f, 3.0f, 4.0f), (float[1])0};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  VertexOutputs v_3 = v_1;
  float v_4[1] = v_3.clipDistance;
  main_outputs v_5 = {v_2.position, v_4[0u]};
  return v_5;
}

