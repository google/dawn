SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn quadSwapDiagonal_2be5e7() -> f16 {
  var arg_0 = 1.0h;
  var res : f16 = quadSwapDiagonal(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_2be5e7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_2be5e7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapDiagonal/2be5e7.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn quadSwapDiagonal_2be5e7() -> f16 {
  var arg_0 = 1.0h;
  var res : f16 = quadSwapDiagonal(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_2be5e7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_2be5e7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapDiagonal/2be5e7.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
