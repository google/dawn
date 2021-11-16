intrinsics/ignore/runtime_array.wgsl:10:5 warning: use of deprecated intrinsic
    ignore(s.arr);
    ^^^^^^

#version 310 es
precision mediump float;


layout (binding = 0) buffer S_1 {
  int arr[];
} s;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  s.arr;
  return;
}
void main() {
  tint_symbol();
}


