enable f16;

fn modf_45005f() {
  var res = modf(vec3<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_45005f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  modf_45005f();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_45005f();
}
