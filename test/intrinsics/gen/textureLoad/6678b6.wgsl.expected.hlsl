intrinsics/gen/textureLoad/6678b6.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<i32> = textureLoad(arg_0, 1);
                       ^^^^^^^^^^^

Texture1D<int4> arg_0 : register(t0, space1);

void textureLoad_6678b6() {
  int4 res = arg_0.Load(int2(1, 0));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureLoad_6678b6();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureLoad_6678b6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_6678b6();
  return;
}
