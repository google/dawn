fn print_41babd() {
  print(vec2<bool>(true));
}

@fragment
fn fragment_main() {
  print_41babd();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_41babd();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_41babd();
  return out;
}
