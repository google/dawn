struct VertexOutputs {
  float clipDistance[3];
  float4 position;
};

struct main_outputs {
  float4 VertexOutputs_position : SV_Position;
  float3 VertexOutputs_clipDistance0 : SV_ClipDistance0;
};


VertexOutputs main_inner() {
  VertexOutputs v = {(float[3])0, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  VertexOutputs v_1 = main_inner();
  VertexOutputs v_2 = v_1;
  float v_3[3] = v_2.clipDistance;
  float v_4[3] = v_2.clipDistance;
  float v_5[3] = v_2.clipDistance;
  VertexOutputs v_6 = v_1;
  main_outputs v_7 = {v_6.position, float3(v_3[0u], v_4[1u], v_5[2u])};
  return v_7;
}

