enable f16;

fn degrees_5e9805() {
  var res : f16 = degrees(f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_5e9805();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_5e9805();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_5e9805();
}
