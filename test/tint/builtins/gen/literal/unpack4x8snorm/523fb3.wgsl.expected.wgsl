fn unpack4x8snorm_523fb3() {
  var res : vec4<f32> = unpack4x8snorm(1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  unpack4x8snorm_523fb3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  unpack4x8snorm_523fb3();
}

@compute @workgroup_size(1)
fn compute_main() {
  unpack4x8snorm_523fb3();
}
