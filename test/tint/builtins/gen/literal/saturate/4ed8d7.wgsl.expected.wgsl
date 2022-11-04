fn saturate_4ed8d7() {
  var res = saturate(vec4(2.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_4ed8d7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_4ed8d7();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_4ed8d7();
}
