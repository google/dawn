Texture2D<float4> ext_tex_plane_1 : register(t1, space1);
cbuffer cbuffer_ext_tex_params : register(b2, space1) {
  uint4 ext_tex_params[17];
};
Texture2D<float4> arg_0 : register(t0, space1);

RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureDimensions_cdc6c9() {
  uint2 res = (ext_tex_params[16].xy + (1u).xx);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureDimensions_cdc6c9();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureDimensions_cdc6c9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_cdc6c9();
  return;
}
