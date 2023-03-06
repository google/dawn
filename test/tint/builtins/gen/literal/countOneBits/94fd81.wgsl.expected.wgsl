fn countOneBits_94fd81() {
  var res : vec2<u32> = countOneBits(vec2<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_94fd81();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countOneBits_94fd81();
}

@compute @workgroup_size(1)
fn compute_main() {
  countOneBits_94fd81();
}
