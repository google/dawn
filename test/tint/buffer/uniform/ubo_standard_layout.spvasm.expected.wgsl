struct inputs_block {
  inner : array<u32, 16u>,
}

@group(0u) @binding(0u) var<uniform> inputs : inputs_block;

@group(0u) @binding(1u) var<storage, read_write> outputs : inputs_block;

@compute @workgroup_size(1u, 1u, 1u)
fn main(@builtin(global_invocation_id) global_invocation_id : vec3<u32>) {
  let idx = global_invocation_id.x;
  outputs.inner[idx] = inputs.inner[idx];
}
