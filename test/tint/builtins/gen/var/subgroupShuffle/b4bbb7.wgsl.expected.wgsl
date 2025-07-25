enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupShuffle_b4bbb7() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var arg_1 = 1u;
  var res : vec3<i32> = subgroupShuffle(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_b4bbb7();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_b4bbb7();
}
