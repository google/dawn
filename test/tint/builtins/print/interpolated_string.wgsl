// [msl] flags: --msl-version 3.2

@compute @workgroup_size(1)
fn compute_main(@builtin(workgroup_id) wgid: vec3u) {
  let a = 10i;
  let b = 20u;
  let c = 30.5f;
  print(`single: ${a}`);
  print(`multiple: a=${a}, b=${b}, c=${c}`);
  print(`expr: ${a + 5i}`);
  print(`adjacent: ${a}${b}`);
  print(`percent: 100% complete: ${wgid.x}`);
}

@fragment
fn fragment_main() {
  print(`frag: ${vec4u(1, 2, 3, 4)}`);
}

struct VertexOutput {
  @builtin(position) pos: vec4<f32>,
};

@vertex
fn vertex_main() -> VertexOutput {
  var out: VertexOutput;
  out.pos = vec4<f32>();
  print(`vert: ${42i}`);
  return out;
}
