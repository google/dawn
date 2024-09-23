SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn quadSwapDiagonal_a82e1d() -> vec3<i32> {
  var res : vec3<i32> = quadSwapDiagonal(vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_a82e1d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_a82e1d();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapDiagonal/a82e1d.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn quadSwapDiagonal_a82e1d() -> vec3<i32> {
  var res : vec3<i32> = quadSwapDiagonal(vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_a82e1d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_a82e1d();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapDiagonal/a82e1d.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
