var<workgroup> a : u32;

var<workgroup> b : u32;

struct S {
  tint_symbol : array<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> buf : S;

@compute @workgroup_size(64u, 1u, 1u)
fn main(@builtin(local_invocation_index) v : u32) {
  if ((v == 0u)) {
    a = buf.tint_symbol[0u];
    b = buf.tint_symbol[1u];
  }
  if (((workgroupUniformLoad(&(a)) + workgroupUniformLoad(&(b))) > 0u)) {
    buf.tint_symbol[v] = 1u;
    workgroupBarrier();
  }
}
