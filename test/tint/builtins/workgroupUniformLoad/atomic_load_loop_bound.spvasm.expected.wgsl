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
  _ = atomicMax(&(g), (*(v_1) & 7u));
  let v_2 = workgroupUniformLoad(&(g));
  {
    var v_3 : u32;
    v_3 = 0u;
    loop {
      let v_4 = v_3;
      if ((v_4 < v_2)) {
        continue;
      } else {
        break;
      }

      continuing {
        *(v_1) = (*(v_1) + v_4);
        workgroupBarrier();
        v_3 = (v_4 + 1u);
      }
    }
  }
}
