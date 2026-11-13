var<workgroup> arr : array<u32, 4u>;

struct S {
  tint_symbol : array<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> buf : S;

@compute @workgroup_size(64u, 1u, 1u)
fn main(@builtin(local_invocation_index) v : u32) {
  if ((v < 4u)) {
    arr[v] = buf.tint_symbol[v];
  }
  if ((workgroupUniformLoad(&(arr[2i])) > 0u)) {
    buf.tint_symbol[v] = 1u;
    workgroupBarrier();
  }
}
