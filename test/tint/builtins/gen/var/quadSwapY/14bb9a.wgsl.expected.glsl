SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn quadSwapY_14bb9a() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  var res : vec4<i32> = quadSwapY(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_14bb9a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_14bb9a();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapY/14bb9a.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn quadSwapY_14bb9a() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  var res : vec4<i32> = quadSwapY(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_14bb9a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_14bb9a();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapY/14bb9a.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
