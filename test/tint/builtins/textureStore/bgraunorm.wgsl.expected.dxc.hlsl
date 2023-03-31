RWTexture2D<float4> tex : register(u0);

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  const float4 value = float4(1.0f, 2.0f, 3.0f, 4.0f);
  tex[int2(9, 8)] = value;
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
