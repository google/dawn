enable subgroups;
requires subgroup_id;

@group(0) @binding(0)
var<storage, read_write> output: array<u32>;

@compute @workgroup_size(64, 1, 1)
fn main(@builtin(num_subgroups) num_subgroups : u32) {
  output[num_subgroups] = num_subgroups;
}
