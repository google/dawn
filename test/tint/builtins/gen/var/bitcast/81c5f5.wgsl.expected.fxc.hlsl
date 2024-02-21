SKIP: FAILED

uint2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 r = f32tof16(float4(src));
  return asuint(uint2((r.x & 0xffff) | ((r.y & 0xffff) << 16), (r.z & 0xffff) | ((r.w & 0xffff) << 16)));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void bitcast_81c5f5() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  uint2 res = tint_bitcast_from_f16(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  bitcast_81c5f5();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  bitcast_81c5f5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  bitcast_81c5f5();
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000016B28511CC0(1,36-44): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x0000016B28511CC0(2,29-31): error X3004: undeclared identifier 'src'
C:\src\dawn\Shader@0x0000016B28511CC0(2,22-32): error X3014: incorrect number of arguments to numeric-type constructor

