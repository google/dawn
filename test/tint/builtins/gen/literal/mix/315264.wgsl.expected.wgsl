fn mix_315264() {
  var res : vec3<f32> = mix(vec3<f32>(1.0f), vec3<f32>(1.0f), 1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_315264();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_315264();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_315264();
}
