enable f16;

fn print_ef160d() {
  var arg_0 = vec3<f16>(1.0h);
  print(arg_0);
}

@fragment
fn fragment_main() {
  print_ef160d();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_ef160d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_ef160d();
  return out;
}
