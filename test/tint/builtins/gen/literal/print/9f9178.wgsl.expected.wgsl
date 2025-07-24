fn print_9f9178() {
  print(true);
}

@fragment
fn fragment_main() {
  print_9f9178();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_9f9178();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_9f9178();
  return out;
}
