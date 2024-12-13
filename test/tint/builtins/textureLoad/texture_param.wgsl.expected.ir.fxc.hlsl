//
// vertex_main
//
struct vertex_main_outputs {
  float4 tint_symbol : SV_Position;
};


Texture2D<int4> arg_0 : register(t0, space1);
int4 textureLoad2d(Texture2D<int4> v, int2 coords, int level) {
  uint3 v_1 = (0u).xxx;
  v.GetDimensions(0u, v_1.x, v_1.y, v_1.z);
  uint v_2 = min(uint(level), (v_1.z - 1u));
  uint3 v_3 = (0u).xxx;
  v.GetDimensions(uint(v_2), v_3.x, v_3.y, v_3.z);
  uint2 v_4 = (v_3.xy - (1u).xx);
  int2 v_5 = int2(min(uint2(coords), v_4));
  return int4(v.Load(int3(v_5, int(v_2))));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (int(0)).xx, int(0));
}

float4 vertex_main_inner() {
  doTextureLoad();
  return (0.0f).xxxx;
}

vertex_main_outputs vertex_main() {
  vertex_main_outputs v_6 = {vertex_main_inner()};
  return v_6;
}

//
// fragment_main
//

Texture2D<int4> arg_0 : register(t0, space1);
int4 textureLoad2d(Texture2D<int4> v, int2 coords, int level) {
  uint3 v_1 = (0u).xxx;
  v.GetDimensions(0u, v_1.x, v_1.y, v_1.z);
  uint v_2 = min(uint(level), (v_1.z - 1u));
  uint3 v_3 = (0u).xxx;
  v.GetDimensions(uint(v_2), v_3.x, v_3.y, v_3.z);
  uint2 v_4 = (v_3.xy - (1u).xx);
  int2 v_5 = int2(min(uint2(coords), v_4));
  return int4(v.Load(int3(v_5, int(v_2))));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (int(0)).xx, int(0));
}

void fragment_main() {
  doTextureLoad();
}

//
// compute_main
//

Texture2D<int4> arg_0 : register(t0, space1);
int4 textureLoad2d(Texture2D<int4> v, int2 coords, int level) {
  uint3 v_1 = (0u).xxx;
  v.GetDimensions(0u, v_1.x, v_1.y, v_1.z);
  uint v_2 = min(uint(level), (v_1.z - 1u));
  uint3 v_3 = (0u).xxx;
  v.GetDimensions(uint(v_2), v_3.x, v_3.y, v_3.z);
  uint2 v_4 = (v_3.xy - (1u).xx);
  int2 v_5 = int2(min(uint2(coords), v_4));
  return int4(v.Load(int3(v_5, int(v_2))));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (int(0)).xx, int(0));
}

[numthreads(1, 1, 1)]
void compute_main() {
  doTextureLoad();
}

