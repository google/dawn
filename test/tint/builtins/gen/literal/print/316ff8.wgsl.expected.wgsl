fn print_316ff8() {
  print(1u);
}

@fragment
fn fragment_main() {
  print_316ff8();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_316ff8();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_316ff8();
  return out;
}
