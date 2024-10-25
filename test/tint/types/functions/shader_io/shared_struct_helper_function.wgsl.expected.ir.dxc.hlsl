struct VertexOutput {
  float4 pos;
  int loc0;
};

struct vert_main1_outputs {
  nointerpolation int VertexOutput_loc0 : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};

struct vert_main2_outputs {
  nointerpolation int VertexOutput_loc0 : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


VertexOutput foo(float x) {
  VertexOutput v = {float4(x, x, x, 1.0f), int(42)};
  return v;
}

VertexOutput vert_main1_inner() {
  VertexOutput v_1 = foo(0.5f);
  return v_1;
}

VertexOutput vert_main2_inner() {
  VertexOutput v_2 = foo(0.25f);
  return v_2;
}

vert_main1_outputs vert_main1() {
  VertexOutput v_3 = vert_main1_inner();
  vert_main1_outputs v_4 = {v_3.loc0, v_3.pos};
  return v_4;
}

vert_main2_outputs vert_main2() {
  VertexOutput v_5 = vert_main2_inner();
  vert_main2_outputs v_6 = {v_5.loc0, v_5.pos};
  return v_6;
}

