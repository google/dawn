Texture2D<int4> arg_0 : register(t0, space1);

int4 textureLoad2d(Texture2D<int4> tint_symbol, int2 coords, int level) {
  return tint_symbol.Load(int3(coords, level));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (0).xx, 0);
}

struct tint_symbol_1 {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  doTextureLoad();
  return (0.0f).xxxx;
}

tint_symbol_1 vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol_1 wrapper_result = (tint_symbol_1)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  doTextureLoad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  doTextureLoad();
  return;
}
