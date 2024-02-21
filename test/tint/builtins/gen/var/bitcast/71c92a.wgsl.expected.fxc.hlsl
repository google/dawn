SKIP: FAILED

vector<float16_t, 4> tint_bitcast_to_f16(int2 src) {
  uint2 v = asuint(src);
  float2 t_low = f16tof32(v & 0xffff);
  float2 t_high = f16tof32((v >> 16) & 0xffff);
  return vector<float16_t, 4>(t_low.x, t_high.x, t_low.y, t_high.y);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void bitcast_71c92a() {
  int2 arg_0 = (1).xx;
  vector<float16_t, 4> res = tint_bitcast_to_f16(arg_0);
  prevent_dce.Store<vector<float16_t, 4> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  bitcast_71c92a();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  bitcast_71c92a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  bitcast_71c92a();
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000022008B91CC0(1,8-16): error X3000: syntax error: unexpected token 'float16_t'

