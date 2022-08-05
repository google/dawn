enable f16;

fn acos_303e3d() {
  var res : f16 = acos(f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_303e3d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_303e3d();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_303e3d();
}
