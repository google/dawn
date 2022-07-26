fn exp2_1f8680() {
  var res : vec3<f32> = exp2(vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_1f8680();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_1f8680();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_1f8680();
}
