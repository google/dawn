fn sqrt_8da177() {
  var res = sqrt(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_8da177();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_8da177();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_8da177();
}
