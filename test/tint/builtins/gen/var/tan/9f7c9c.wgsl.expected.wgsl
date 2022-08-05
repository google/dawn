enable f16;

fn tan_9f7c9c() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = tan(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_9f7c9c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_9f7c9c();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_9f7c9c();
}
