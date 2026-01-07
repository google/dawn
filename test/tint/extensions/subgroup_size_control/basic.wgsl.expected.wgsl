enable subgroups;
enable chromium_experimental_subgroup_size_control;

@group(0) @binding(0) var<storage, read_write> buffer : array<u32>;

@compute @workgroup_size(32, 1, 1) @subgroup_size(32)
fn main(@builtin(subgroup_invocation_id) sg_id : u32, @builtin(subgroup_size) sg_size : u32) {
  buffer[sg_id] = sg_size;
}
