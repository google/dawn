enable f16;

struct SB_RW {
  arg_0 : array<f16>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn arrayLength_cbd6b5() {
  var res : u32 = arrayLength(&(sb_rw.arg_0));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  arrayLength_cbd6b5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  arrayLength_cbd6b5();
}

@compute @workgroup_size(1)
fn compute_main() {
  arrayLength_cbd6b5();
}
