fn print_53dca8() {
  print(1i);
}

@fragment
fn fragment_main() {
  print_53dca8();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_53dca8();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_53dca8();
  return out;
}
