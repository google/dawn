SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupAdd_28db2c() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  var res : vec4<i32> = subgroupAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_28db2c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_28db2c();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupAdd/28db2c.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupAdd_28db2c() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  var res : vec4<i32> = subgroupAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_28db2c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_28db2c();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupAdd/28db2c.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
