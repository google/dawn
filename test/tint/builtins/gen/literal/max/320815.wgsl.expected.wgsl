fn max_320815() {
  var res : vec2<u32> = max(vec2<u32>(1u), vec2<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_320815();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_320815();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_320815();
}
