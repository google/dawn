SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> quadBroadcast_0464d1() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  vector<float16_t, 2> res = QuadReadLaneAt(arg_0, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, quadBroadcast_0464d1());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, quadBroadcast_0464d1());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000021E98773EE0(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

