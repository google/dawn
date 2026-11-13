var<workgroup> g : u32;

struct S {
  tint_symbol : array<u32>,
}

@group(0u) @binding(0u) var<storage, read_write> buf : S;

@compute @workgroup_size(64u, 1u, 1u)
fn main(@builtin(local_invocation_index) v : u32) {
  var v_1 : u32;
  v_1 = v;
  v_2(&(v_1));
}

fn v_2(idx : ptr<function, u32>) {
  var b : bool;
  if ((*(idx) == 0u)) {
    g = buf.tint_symbol[0u];
  }
  b = (workgroupUniformLoad(&(g)) > 4u);
  if (b) {
    buf.tint_symbol[*(idx)] = 1u;
    workgroupBarrier();
  }
}
