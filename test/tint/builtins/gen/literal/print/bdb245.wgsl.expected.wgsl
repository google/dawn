fn print_bdb245() {
  print(vec2<f32>(1.0f));
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
