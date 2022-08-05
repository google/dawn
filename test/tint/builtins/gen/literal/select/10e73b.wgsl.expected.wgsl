enable f16;

fn select_10e73b() {
  var res : f16 = select(f16(), f16(), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_10e73b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_10e73b();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_10e73b();
}
