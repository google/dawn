enable f16;

fn degrees_f59715() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = degrees(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_f59715();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_f59715();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_f59715();
}
