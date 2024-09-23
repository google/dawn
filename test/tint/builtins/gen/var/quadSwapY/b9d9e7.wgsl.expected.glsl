SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn quadSwapY_b9d9e7() -> vec4<f32> {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = quadSwapY(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_b9d9e7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_b9d9e7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapY/b9d9e7.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn quadSwapY_b9d9e7() -> vec4<f32> {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = quadSwapY(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_b9d9e7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_b9d9e7();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapY/b9d9e7.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
