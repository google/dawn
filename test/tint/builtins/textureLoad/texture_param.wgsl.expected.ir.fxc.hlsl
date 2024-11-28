struct vertex_main_outputs {
  float4 tint_symbol_1 : SV_Position;
};


Texture2D<int4> arg_0 : register(t0, space1);
int4 textureLoad2d(Texture2D<int4> tint_symbol, int2 coords, int level) {
  uint3 v = (0u).xxx;
  tint_symbol.GetDimensions(0u, v.x, v.y, v.z);
  uint v_1 = min(uint(level), (v.z - 1u));
  uint3 v_2 = (0u).xxx;
  tint_symbol.GetDimensions(uint(v_1), v_2.x, v_2.y, v_2.z);
  uint2 v_3 = (v_2.xy - (1u).xx);
  int2 v_4 = int2(min(uint2(coords), v_3));
  return int4(tint_symbol.Load(int3(v_4, int(v_1))));
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
  vertex_main_outputs v_5 = {vertex_main_inner()};
  return v_5;
}

