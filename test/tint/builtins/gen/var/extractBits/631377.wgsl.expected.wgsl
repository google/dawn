fn extractBits_631377() {
  var arg_0 = vec4<u32>(1u);
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res : vec4<u32> = extractBits(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_631377();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  extractBits_631377();
}

@compute @workgroup_size(1)
fn compute_main() {
  extractBits_631377();
}
