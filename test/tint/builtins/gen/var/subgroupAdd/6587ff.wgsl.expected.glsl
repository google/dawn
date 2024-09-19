SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupAdd_6587ff() -> vec3<u32> {
  var arg_0 = vec3<u32>(1u);
  var res : vec3<u32> = subgroupAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_6587ff();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_6587ff();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupAdd/6587ff.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupAdd_6587ff() -> vec3<u32> {
  var arg_0 = vec3<u32>(1u);
  var res : vec3<u32> = subgroupAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_6587ff();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_6587ff();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupAdd/6587ff.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
