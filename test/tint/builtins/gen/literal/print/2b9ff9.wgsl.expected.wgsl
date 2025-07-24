fn print_2b9ff9() {
  print(1.0f);
}

@fragment
fn fragment_main() {
  print_2b9ff9();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_2b9ff9();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_2b9ff9();
  return out;
}
