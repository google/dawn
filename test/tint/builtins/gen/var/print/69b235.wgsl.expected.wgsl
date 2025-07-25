enable f16;

fn print_69b235() {
  var arg_0 = vec4<f16>(1.0h);
  print(arg_0);
}

@fragment
fn fragment_main() {
  print_69b235();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_69b235();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_69b235();
  return out;
}
