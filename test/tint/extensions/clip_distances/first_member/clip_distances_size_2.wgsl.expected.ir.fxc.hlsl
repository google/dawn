struct VertexOutputs {
  float clipDistance[2];
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float2 VertexOutputs_clipDistance0 : SV_ClipDistance0;
};


VertexOutputs main_inner() {
  VertexOutputs v = {(float[2])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  float v_3[2] = v_2.clipDistance;
  float v_4[2] = v_2.clipDistance;
  VertexOutputs v_5 = v_1;
  main_outputs v_6 = {v_5.position, float2(v_3[0u], v_4[1u])};
  return v_6;
}

