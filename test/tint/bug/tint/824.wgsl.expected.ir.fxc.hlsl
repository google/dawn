struct Output {
  float4 Position;
  float4 color;
};

struct main_outputs {
  float4 Output_color : TEXCOORD0;
  float4 Output_Position : SV_Position;
};

struct main_inputs {
  uint VertexIndex : SV_VertexID;
  uint InstanceIndex : SV_InstanceID;
};


Output main_inner(uint VertexIndex, uint InstanceIndex) {
  float2 v[4] = {(0.20000000298023223877f).xx, (0.30000001192092895508f).xx, (-0.10000000149011611938f).xx, (1.10000002384185791016f).xx};
  float2 zv[4] = v;
  float z = zv[InstanceIndex][0u];
  Output output = (Output)0;
  output.Position = float4(0.5f, 0.5f, z, 1.0f);
  float4 v_1[4] = {float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), (1.0f).xxxx};
  float4 colors[4] = v_1;
  output.color = colors[InstanceIndex];
  Output v_2 = output;
  return v_2;
}

main_outputs main(main_inputs inputs) {
  Output v_3 = main_inner(inputs.VertexIndex, inputs.InstanceIndex);
  Output v_4 = v_3;
  Output v_5 = v_3;
  main_outputs v_6 = {v_5.color, v_4.Position};
  return v_6;
}

