enable f16;

fn bitcast_a58b50() {
  var res : u32 = bitcast<u32>(vec2<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_a58b50();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_a58b50();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_a58b50();
}
