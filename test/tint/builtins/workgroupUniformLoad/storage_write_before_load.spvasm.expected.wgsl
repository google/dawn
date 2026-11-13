var<workgroup> g : u32;

struct S {
  tint_symbol : array<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> buf : S;

@compute @workgroup_size(64u, 1u, 1u)
fn main(@builtin(local_invocation_index) v : u32) {
  if ((v == 0u)) {
    g = buf.tint_symbol[0u];
  }
  let v_1 = &(buf.tint_symbol[v]);
  *(v_1) = 2u;
  if ((workgroupUniformLoad(&(g)) > 4u)) {
    *(v_1) = 1u;
    workgroupBarrier();
  }
}
