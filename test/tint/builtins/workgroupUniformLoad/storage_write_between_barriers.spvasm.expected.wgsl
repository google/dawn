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
  let v_1 = workgroupUniformLoad(&(g));
  let v_2 = &(buf.tint_symbol[v]);
  *(v_2) = v_1;
  if ((v_1 > 4u)) {
    *(v_2) = 1u;
    workgroupBarrier();
  }
}
