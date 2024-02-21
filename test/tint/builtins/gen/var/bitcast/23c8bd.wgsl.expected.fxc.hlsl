SKIP: FAILED

float tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 r = f32tof16(float2(src));
  return asfloat(uint((r.x & 0xffff) | ((r.y & 0xffff) << 16)));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void bitcast_23c8bd() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  float res = tint_bitcast_from_f16(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  bitcast_23c8bd();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  bitcast_23c8bd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  bitcast_23c8bd();
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002397E150600(1,36-44): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x000002397E150600(2,29-31): error X3004: undeclared identifier 'src'
C:\src\dawn\Shader@0x000002397E150600(2,22-32): error X3014: incorrect number of arguments to numeric-type constructor

