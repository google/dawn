SKIP: INVALID


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupBallot_1a8251() -> vec4<u32> {
  var arg_0 = true;
  var res : vec4<u32> = subgroupBallot(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBallot_1a8251();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBallot_1a8251();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBallot/1a8251.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupBallot_1a8251() -> vec4<u32> {
  var arg_0 = true;
  var res : vec4<u32> = subgroupBallot(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBallot_1a8251();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBallot_1a8251();
}

Failed to generate: <dawn>/test/tint/builtins/gen/var/subgroupBallot/1a8251.wgsl:41:8 error: GLSL backend does not support extension 'subgroups'
enable subgroups;
       ^^^^^^^^^


tint executable returned error: exit status 1
