SKIP: INVALID


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupAdd_0dd12a() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var res : vec3<f16> = subgroupAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_0dd12a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_0dd12a();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupAdd/0dd12a.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;
enable subgroups_f16;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn subgroupAdd_0dd12a() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  var res : vec3<f16> = subgroupAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_0dd12a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_0dd12a();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupAdd/0dd12a.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
