intrinsics/ignore/storage_buffer.wgsl:10:5 warning: use of deprecated intrinsic
    ignore(s);
    ^^^^^^

intrinsics/ignore/storage_buffer.wgsl:11:5 warning: use of deprecated intrinsic
    ignore(s.i);
    ^^^^^^

#version 310 es
precision mediump float;


layout (binding = 0) buffer S_1 {
  int i;
} s;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  s;
  s.i;
  return;
}
void main() {
  tint_symbol();
}


