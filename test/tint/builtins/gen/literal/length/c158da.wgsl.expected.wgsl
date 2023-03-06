enable f16;

fn length_c158da() {
  var res : f16 = length(0.0h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_c158da();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_c158da();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_c158da();
}
