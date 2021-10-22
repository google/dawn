intrinsics/ignore/storage_buffer.wgsl:10:5 warning: use of deprecated intrinsic
    ignore(s);
    ^^^^^^

intrinsics/ignore/storage_buffer.wgsl:11:5 warning: use of deprecated intrinsic
    ignore(s.i);
    ^^^^^^

RWByteAddressBuffer s : register(u0, space0);

[numthreads(1, 1, 1)]
void main() {
  s;
  s;
  return;
}
