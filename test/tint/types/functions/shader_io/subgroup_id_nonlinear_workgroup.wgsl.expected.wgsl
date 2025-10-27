enable subgroups;
requires subgroup_id;

@group(0) @binding(0) var<storage, read_write> output : array<u32>;

@compute @workgroup_size(8, 8, 1)
fn main(@builtin(subgroup_id) subgroup_id : u32) {
  output[subgroup_id] = subgroup_id;
}
