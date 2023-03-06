fn min_03c7e3() {
  var res : vec2<i32> = min(vec2<i32>(1i), vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_03c7e3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_03c7e3();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_03c7e3();
}
