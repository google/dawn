SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn quadSwapY_0d05a8() -> vec2<i32> {
  var res : vec2<i32> = quadSwapY(vec2<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_0d05a8();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_0d05a8();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapY/0d05a8.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn quadSwapY_0d05a8() -> vec2<i32> {
  var res : vec2<i32> = quadSwapY(vec2<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_0d05a8();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_0d05a8();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapY/0d05a8.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
