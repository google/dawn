SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int quadSwapX_1e1086() {
  int arg_0 = 1;
  int res = QuadReadAcrossX(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_1e1086()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadSwapX_1e1086()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000025C71A79B20(5,13-34): error X3004: undeclared identifier 'QuadReadAcrossX'

