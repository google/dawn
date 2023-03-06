enable f16;

fn radians_208fd9() {
  var res : f16 = radians(1.0h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_208fd9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_208fd9();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_208fd9();
}
