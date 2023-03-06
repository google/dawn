fn acos_dfc915() {
  var res : vec2<f32> = acos(vec2<f32>(0.96891242265701293945f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_dfc915();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_dfc915();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_dfc915();
}
