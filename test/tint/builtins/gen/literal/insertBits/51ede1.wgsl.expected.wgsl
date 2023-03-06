fn insertBits_51ede1() {
  var res : vec4<u32> = insertBits(vec4<u32>(1u), vec4<u32>(1u), 1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  insertBits_51ede1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  insertBits_51ede1();
}

@compute @workgroup_size(1)
fn compute_main() {
  insertBits_51ede1();
}
