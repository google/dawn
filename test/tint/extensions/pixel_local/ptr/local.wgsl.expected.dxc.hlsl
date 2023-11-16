RasterizerOrderedTexture2D<uint4> pixel_local_a : register(u1);

struct PixelLocal {
  uint a;
};

static PixelLocal V = (PixelLocal)0;

void load_from_pixel_local_storage(float4 my_input) {
  const uint2 rov_texcoord = uint2(my_input.xy);
  V.a = pixel_local_a.Load(rov_texcoord).x;
}

void store_into_pixel_local_storage(float4 my_input) {
  const uint2 rov_texcoord = uint2(my_input.xy);
  pixel_local_a[rov_texcoord] = uint4((V.a).xxxx);
}

struct tint_symbol_1 {
  float4 my_pos : SV_Position;
};

void f_inner() {
  V.a = 42u;
}

void f_inner_1(float4 my_pos) {
  const float4 hlsl_sv_position = my_pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  f_inner();
  store_into_pixel_local_storage(hlsl_sv_position);
}

void f(tint_symbol_1 tint_symbol) {
  f_inner_1(tint_symbol.my_pos);
  return;
}
