fn extractBits_f28f69() {
  var arg_0 = vec2<u32>(1u);
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res : vec2<u32> = extractBits(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_f28f69();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  extractBits_f28f69();
}

@compute @workgroup_size(1)
fn compute_main() {
  extractBits_f28f69();
}
