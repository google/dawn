var<workgroup> g : atomic<u32>;

struct S {
  tint_symbol : array<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> buf : S;

@compute @workgroup_size(64u, 1u, 1u)
fn main(@builtin(local_invocation_index) v : u32) {
  if ((v == 0u)) {
    atomicStore(&(g), 0u);
  }
  workgroupBarrier();
  let v_1 = &(buf.tint_symbol[v]);
  _ = atomicMax(&(g), *(v_1));
  if ((workgroupUniformLoad(&(g)) > 4u)) {
    *(v_1) = 1u;
    workgroupBarrier();
  }
}
