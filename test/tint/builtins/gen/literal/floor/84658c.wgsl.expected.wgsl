enable f16;

fn floor_84658c() {
  var res : vec2<f16> = floor(vec2<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_84658c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_84658c();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_84658c();
}
