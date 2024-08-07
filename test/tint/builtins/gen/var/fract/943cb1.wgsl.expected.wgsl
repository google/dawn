@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn fract_943cb1() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.25f);
  var res : vec2<f32> = fract(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fract_943cb1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = fract_943cb1();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = fract_943cb1();
  return out;
}
