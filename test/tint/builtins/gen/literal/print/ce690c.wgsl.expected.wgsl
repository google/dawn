fn print_ce690c() {
  print(vec2<u32>(1u));
}

@fragment
fn fragment_main() {
  print_ce690c();
}

@compute @workgroup_size(1)
fn compute_main() {
  print_ce690c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  print_ce690c();
  return out;
}
