struct PixelLocal {
  uint a;
  int b;
  float c;
};

struct In {
  float4 pos;
  bool ff;
  uint si;
};

struct f_inputs {
  float4 In_pos : SV_Position;
  bool In_ff : SV_IsFrontFace;
  uint In_si : SV_SampleIndex;
};


static PixelLocal P = (PixelLocal)0;
RasterizerOrderedTexture2D<uint4> pixel_local_a : register(u1);
RasterizerOrderedTexture2D<int4> pixel_local_b : register(u6);
RasterizerOrderedTexture2D<float4> pixel_local_c : register(u3);
uint tint_f32_to_u32(float value) {
  return (((value <= 4294967040.0f)) ? ((((value >= 0.0f)) ? (uint(value)) : (0u))) : (4294967295u));
}

void f_inner(In tint_symbol) {
  uint v = tint_f32_to_u32(tint_symbol.pos[0u]);
  P.a = (P.a + v);
}

void f(f_inputs inputs) {
  uint2 v_1 = uint2(inputs.In_pos.xy);
  P.a = pixel_local_a.Load(v_1).x;
  P.b = pixel_local_b.Load(v_1).x;
  P.c = pixel_local_c.Load(v_1).x;
  In v_2 = {float4(inputs.In_pos.xyz, (1.0f / inputs.In_pos[3u])), inputs.In_ff, inputs.In_si};
  f_inner(v_2);
  pixel_local_a[v_1] = P.a.xxxx;
  pixel_local_b[v_1] = P.b.xxxx;
  pixel_local_c[v_1] = P.c.xxxx;
}

