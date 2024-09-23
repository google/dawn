SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn quadSwapY_d1ab4d() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = quadSwapY(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_d1ab4d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_d1ab4d();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapY/d1ab4d.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn quadSwapY_d1ab4d() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = quadSwapY(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapY_d1ab4d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapY_d1ab4d();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapY/d1ab4d.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
