SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn quadSwapY_06a67c() -> vec3<u32> {
  var res : vec3<u32> = quadSwapY(vec3<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_06a67c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_06a67c();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapY/06a67c.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn quadSwapY_06a67c() -> vec3<u32> {
  var res : vec3<u32> = quadSwapY(vec3<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_06a67c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_06a67c();
}

Failed to generate: <dawn>/test/tint/builtins/gen/literal/quadSwapY/06a67c.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
