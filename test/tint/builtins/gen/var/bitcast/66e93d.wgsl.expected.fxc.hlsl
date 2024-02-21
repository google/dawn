SKIP: FAILED

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = asuint(src);
  float t_low = f16tof32(v & 0xffff);
  float t_high = f16tof32((v >> 16) & 0xffff);
  return vector<float16_t, 2>(t_low.x, t_high.x);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void bitcast_66e93d() {
  uint arg_0 = 1u;
  vector<float16_t, 2> res = tint_bitcast_to_f16(arg_0);
  prevent_dce.Store<vector<float16_t, 2> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  bitcast_66e93d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  bitcast_66e93d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  bitcast_66e93d();
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002234AD11CC0(1,8-16): error X3000: syntax error: unexpected token 'float16_t'

