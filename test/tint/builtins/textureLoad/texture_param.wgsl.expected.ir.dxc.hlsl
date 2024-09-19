struct vertex_main_outputs {
  float4 tint_symbol_1 : SV_Position;
};


Texture2D<int4> arg_0 : register(t0, space1);
int4 textureLoad2d(Texture2D<int4> tint_symbol, int2 coords, int level) {
  int2 v = int2(coords);
  return int4(tint_symbol.Load(int3(v, int(level))));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (int(0)).xx, int(0));
}

float4 vertex_main_inner() {
  doTextureLoad();
  return (0.0f).xxxx;
}

void fragment_main() {
  doTextureLoad();
}

[numthreads(1, 1, 1)]
void compute_main() {
  doTextureLoad();
}

vertex_main_outputs vertex_main() {
  vertex_main_outputs v_1 = {vertex_main_inner()};
  return v_1;
}

