enable f16;

fn print_65e661() {
  print(vec2<f16>(1.0h));
}

@fragment
fn fragment_main() {
  print_65e661();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_65e661();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_65e661();
  return out;
}
