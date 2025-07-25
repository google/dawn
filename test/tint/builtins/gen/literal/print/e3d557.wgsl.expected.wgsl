fn print_e3d557() {
  print(vec4<bool>(true));
}

@fragment
fn fragment_main() {
  print_e3d557();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_e3d557();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_e3d557();
  return out;
}
