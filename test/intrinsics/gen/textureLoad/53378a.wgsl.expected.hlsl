intrinsics/gen/textureLoad/53378a.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<i32> = textureLoad(arg_0, vec2<i32>());
                       ^^^^^^^^^^^

Texture2D<int4> arg_0 : register(t0, space1);

void textureLoad_53378a() {
  int4 res = arg_0.Load(int3(0, 0, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  textureLoad_53378a();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  textureLoad_53378a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_53378a();
  return;
}
