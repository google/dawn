fn clamp_6c1749() {
  var res : vec2<i32> = clamp(vec2<i32>(1i), vec2<i32>(1i), vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_6c1749();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_6c1749();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_6c1749();
}
