intrinsics/ignore/runtime_array.wgsl:9:5 warning: use of deprecated intrinsic
    ignore(s.arr);
    ^^^^^^

RWByteAddressBuffer s : register(u0, space0);

[numthreads(1, 1, 1)]
void main() {
  s;
  return;
}
