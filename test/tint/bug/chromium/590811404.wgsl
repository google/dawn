// flags: --polyfill-nested-aggregate-construct

struct S {
    a : u32,
    b : u32,
    c : u32,
}
struct T {
    x : S,
    y : S,
    z : S,
}
struct In {
    @builtin(global_invocation_id) gid : vec3<u32>,
}

@compute @workgroup_size(1)
fn main(inp : In) {
    var s : S;
    let v = s;
    let k = i32(f32(v.a));
    switch k {
        case 2144158127: {
            let t = S(v.a, v.a, v.a);
            let u = T(t, t, t);
        }
        default: {}
    }
}
