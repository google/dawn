fn pow_e42f20() {
  var res = pow(vec3(1.0), vec3(1.0));
}

@fragment
fn fragment_main() {
  pow_e42f20();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_e42f20();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  pow_e42f20();
  return out;
}
