enable chromium_experimental_dp4a;

fn dot4I8Packed_881e62() {
  var res : i32 = dot4I8Packed(1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot4I8Packed_881e62();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot4I8Packed_881e62();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot4I8Packed_881e62();
}
