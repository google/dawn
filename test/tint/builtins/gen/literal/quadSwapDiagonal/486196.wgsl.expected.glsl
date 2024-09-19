SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn quadSwapDiagonal_486196() -> f32 {
  var res : f32 = quadSwapDiagonal(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_486196();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_486196();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapDiagonal/486196.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn quadSwapDiagonal_486196() -> f32 {
  var res : f32 = quadSwapDiagonal(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_486196();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_486196();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapDiagonal/486196.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
