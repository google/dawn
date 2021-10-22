intrinsics/ignore/uniform_buffer.wgsl:10:5 warning: use of deprecated intrinsic
    ignore(u);
    ^^^^^^

intrinsics/ignore/uniform_buffer.wgsl:11:5 warning: use of deprecated intrinsic
    ignore(u.i);
    ^^^^^^

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[1];
};

[numthreads(1, 1, 1)]
void main() {
  u;
  u;
  return;
}
