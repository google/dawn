enable f16;

fn print_d07705() {
  print(1.0h);
}

@fragment
fn fragment_main() {
  print_d07705();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_d07705();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_d07705();
  return out;
}
