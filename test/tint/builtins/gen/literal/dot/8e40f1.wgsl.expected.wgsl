enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn dot_8e40f1() -> f16 {
  var res : f16 = dot(vec3<f16>(1.0h), vec3<f16>(1.0h));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dot_8e40f1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = dot_8e40f1();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f16,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = dot_8e40f1();
  return out;
}
