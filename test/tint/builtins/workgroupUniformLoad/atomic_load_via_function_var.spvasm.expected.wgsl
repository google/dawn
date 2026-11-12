var<workgroup> g : atomic<u32>;

struct S {
  tint_symbol : array<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> buf : S;

struct S_1 {
  p : u32,
}

@group(0u) @binding(1u) var<uniform> Params : S_1;

@compute @workgroup_size(64u, 1u, 1u)
fn main(@builtin(local_invocation_index) v : u32) {
  var v_1 : u32;
  if ((Params.p != 0u)) {
    if ((v == 0u)) {
      atomicStore(&(g), 0u);
    }
    workgroupBarrier();
    _ = atomicMax(&(g), buf.tint_symbol[v]);
    v_1 = workgroupUniformLoad(&(g));
  } else {
    v_1 = 0u;
  }
  if ((v_1 > 4u)) {
    buf.tint_symbol[v] = 1u;
    workgroupBarrier();
  }
}
