struct PixelLocal {
  uint a;
  int b;
  float c;
};

struct f_inputs {
  float4 pos : SV_Position;
};

struct f2_inputs {
  float4 pos : SV_Position;
};

struct f3_inputs {
  float4 pos : SV_Position;
};


static PixelLocal P = (PixelLocal)0;
RasterizerOrderedTexture2D<uint4> pixel_local_a : register(u1);
RasterizerOrderedTexture2D<int4> pixel_local_b : register(u6);
RasterizerOrderedTexture2D<float4> pixel_local_c : register(u3);
uint tint_f32_to_u32(float value) {
  return (((value <= 4294967040.0f)) ? ((((value >= 0.0f)) ? (uint(value)) : (0u))) : (4294967295u));
}

void f_inner(float4 pos) {
  uint v = tint_f32_to_u32(pos[0u]);
  P.a = (P.a + v);
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

void f2_inner(float4 pos) {
  int v_1 = tint_f32_to_i32(pos[0u]);
  P.b = (P.b + v_1);
}

void f3_inner(float4 pos) {
  P.c = (P.c + pos[0u]);
}

void f(f_inputs inputs) {
  uint2 v_2 = uint2(inputs.pos.xy);
  P.a = pixel_local_a.Load(v_2).x;
  P.b = pixel_local_b.Load(v_2).x;
  P.c = pixel_local_c.Load(v_2).x;
  f_inner(float4(inputs.pos.xyz, (1.0f / inputs.pos[3u])));
  pixel_local_a[v_2] = P.a.xxxx;
  pixel_local_b[v_2] = P.b.xxxx;
  pixel_local_c[v_2] = P.c.xxxx;
}

void f2(f2_inputs inputs) {
  uint2 v_3 = uint2(inputs.pos.xy);
  P.a = pixel_local_a.Load(v_3).x;
  P.b = pixel_local_b.Load(v_3).x;
  P.c = pixel_local_c.Load(v_3).x;
  f2_inner(float4(inputs.pos.xyz, (1.0f / inputs.pos[3u])));
  pixel_local_a[v_3] = P.a.xxxx;
  pixel_local_b[v_3] = P.b.xxxx;
  pixel_local_c[v_3] = P.c.xxxx;
}

void f3(f3_inputs inputs) {
  uint2 v_4 = uint2(inputs.pos.xy);
  P.a = pixel_local_a.Load(v_4).x;
  P.b = pixel_local_b.Load(v_4).x;
  P.c = pixel_local_c.Load(v_4).x;
  f3_inner(float4(inputs.pos.xyz, (1.0f / inputs.pos[3u])));
  pixel_local_a[v_4] = P.a.xxxx;
  pixel_local_b[v_4] = P.b.xxxx;
  pixel_local_c[v_4] = P.c.xxxx;
}

