SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn quadSwapX_9bea80() -> f32 {
  var arg_0 = 1.0f;
  var res : f32 = quadSwapX(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_9bea80();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_9bea80();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapX/9bea80.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn quadSwapX_9bea80() -> f32 {
  var arg_0 = 1.0f;
  var res : f32 = quadSwapX(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_9bea80();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_9bea80();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapX/9bea80.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
