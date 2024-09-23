SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn quadSwapX_053f3b() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = quadSwapX(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_053f3b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_053f3b();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapX/053f3b.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn quadSwapX_053f3b() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = quadSwapX(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_053f3b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_053f3b();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/quadSwapX/053f3b.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
