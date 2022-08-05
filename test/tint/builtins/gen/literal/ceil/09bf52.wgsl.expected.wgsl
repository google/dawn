enable f16;

fn ceil_09bf52() {
  var res : vec3<f16> = ceil(vec3<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_09bf52();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_09bf52();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_09bf52();
}
