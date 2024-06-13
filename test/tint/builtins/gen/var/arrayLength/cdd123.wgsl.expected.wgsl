struct SB_RW {
  arg_0 : array<f32>,
}

@group(0) @binding(1) var<storage, read_write> sb_rw : SB_RW;

fn arrayLength_cdd123() -> u32 {
  var res : u32 = arrayLength(&(sb_rw.arg_0));
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@fragment
fn fragment_main() {
  prevent_dce = arrayLength_cdd123();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = arrayLength_cdd123();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : u32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = arrayLength_cdd123();
  return out;
}
