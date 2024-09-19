SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn quadSwapDiagonal_a090b0() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = quadSwapDiagonal(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_a090b0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_a090b0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapDiagonal/a090b0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn quadSwapDiagonal_a090b0() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = quadSwapDiagonal(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapDiagonal_a090b0();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapDiagonal_a090b0();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapDiagonal/a090b0.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
