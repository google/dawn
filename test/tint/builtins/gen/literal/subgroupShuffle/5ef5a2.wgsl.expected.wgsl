enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupShuffle_5ef5a2() -> vec2<u32> {
  var res : vec2<u32> = subgroupShuffle(vec2<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_5ef5a2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_5ef5a2();
}
