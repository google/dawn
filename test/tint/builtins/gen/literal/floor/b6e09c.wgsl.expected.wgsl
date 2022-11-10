enable f16;

fn floor_b6e09c() {
  var res : f16 = floor(1.5h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_b6e09c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_b6e09c();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_b6e09c();
}
