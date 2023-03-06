fn extractBits_fb850f() {
  var res : vec4<i32> = extractBits(vec4<i32>(1i), 1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_fb850f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  extractBits_fb850f();
}

@compute @workgroup_size(1)
fn compute_main() {
  extractBits_fb850f();
}
