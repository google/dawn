struct SB_RO {
  arg_0 : array<i32>,
}

@group(0) @binding(1) var<storage, read> sb_ro : SB_RO;

fn arrayLength_1588cd() {
  var res : u32 = arrayLength(&(sb_ro.arg_0));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  arrayLength_1588cd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  arrayLength_1588cd();
}

@compute @workgroup_size(1)
fn compute_main() {
  arrayLength_1588cd();
}
