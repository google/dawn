intrinsics/ignore/uniform_buffer.wgsl:10:5 warning: use of deprecated intrinsic
    ignore(u);
    ^^^^^^

intrinsics/ignore/uniform_buffer.wgsl:11:5 warning: use of deprecated intrinsic
    ignore(u.i);
    ^^^^^^

#version 310 es
precision mediump float;


layout (binding = 0) uniform S_1 {
  int i;
} u;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  u;
  u.i;
  return;
}
void main() {
  tint_symbol();
}


