SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn quadSwapDiagonal_9ccb38() -> i32 {
  var res : i32 = quadSwapDiagonal(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_9ccb38();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_9ccb38();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapDiagonal/9ccb38.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn quadSwapDiagonal_9ccb38() -> i32 {
  var res : i32 = quadSwapDiagonal(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_9ccb38();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_9ccb38();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapDiagonal/9ccb38.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
