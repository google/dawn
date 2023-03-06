struct SB_RW {
  arg_0 : array<u32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn arrayLength_eb510f() {
  var res : u32 = arrayLength(&(sb_rw.arg_0));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  arrayLength_eb510f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  arrayLength_eb510f();
}

@compute @workgroup_size(1)
fn compute_main() {
  arrayLength_eb510f();
}
