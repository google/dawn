Texture2DArray<float4> arg_0 : register(t0, space1);

void textureLoad_87be85() {
  float4 res = arg_0.Load(int4(0, 0, 1, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureLoad_87be85();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureLoad_87be85();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_87be85();
  return;
}
