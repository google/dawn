fn print_bdb245() {
  var arg_0 = vec2<f32>(1.0f);
  print(arg_0);
}

@fragment
fn fragment_main() {
  print_bdb245();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_bdb245();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_bdb245();
  return out;
}
