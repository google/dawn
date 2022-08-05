enable f16;

fn radians_fbacf0() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = radians(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_fbacf0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_fbacf0();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_fbacf0();
}
