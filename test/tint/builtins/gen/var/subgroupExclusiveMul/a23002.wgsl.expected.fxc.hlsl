SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupExclusiveMul_a23002() {
  int arg_0 = 1;
  int res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_a23002()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002022064D400(5,13-36): error X3004: undeclared identifier 'WavePrefixProduct'

