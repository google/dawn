struct SB {
  data : array<i32>,
};

@group(0) @binding(0) var<storage, read_write> buffer : SB;

@stage(compute) @workgroup_size(1, 2, 3)
fn main(@builtin(global_invocation_id) id : vec3<u32>) {
  buffer.data[id.x] = buffer.data[id.x] + 1;
}
