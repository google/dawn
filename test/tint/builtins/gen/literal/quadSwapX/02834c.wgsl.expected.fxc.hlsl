SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> quadSwapX_02834c() {
  vector<float16_t, 4> res = QuadReadAcrossX((float16_t(1.0h)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, quadSwapX_02834c());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, quadSwapX_02834c());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000028CCC2246D0(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

