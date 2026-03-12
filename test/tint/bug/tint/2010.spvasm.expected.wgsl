struct tint_symbol_2 {
  tint_symbol : vec2<f32>,
  tint_symbol_1 : u32,
}

var<workgroup> v : array<tint_symbol_2, 4096u>;

var<workgroup> v_1 : atomic<u32>;

var<workgroup> v_2 : atomic<u32>;

var<workgroup> v_3 : atomic<u32>;

var<workgroup> v_4 : atomic<u32>;

struct tint_symbol_4 {
  tint_symbol_3 : u32,
}

struct tint_symbol_6 {
  tint_symbol_5 : tint_symbol_4,
}

@group(0u) @binding(1u) var<uniform> v_5 : tint_symbol_6;

struct tint_symbol_8 {
  tint_symbol_7 : array<vec4<f32>>,
}

@group(0u) @binding(2u) var<storage, read> v_6 : tint_symbol_8;

struct tint_symbol_10 {
  tint_symbol_9 : array<vec4<f32>>,
}

@group(0u) @binding(3u) var<storage, read_write> v_7 : tint_symbol_10;

@compute @workgroup_size(32u, 1u, 1u)
fn main(@builtin(local_invocation_id) v_8 : vec3<u32>) {
  let v_9 = v_8.x;
  var v_10 : u32;
  {
    var v_11 : u32;
    v_11 = 0u;
    loop {
      let v_12 = v_11;
      let v_13 = v_5.tint_symbol_5.tint_symbol_3;
      if ((v_12 < v_13)) {
        let v_14 = (v_12 + v_9);
        if ((v_14 >= v_13)) {
          let v_15 = v_6.tint_symbol_7[v_14];
          v[v_14] = tint_symbol_2(((v_15.xy + v_15.zw) * 0.5f), v_14);
        }
        continue;
      } else {
        v_10 = v_13;
        break;
      }

      continuing {
        v_11 = (v_12 + 32u);
      }
    }
  }
  let v_16 = v_10;
  workgroupBarrier();
  let v_17 = bitcast<i32>(v_16);
  let v_18 = v[0i].tint_symbol;
  if ((v_9 == 0u)) {
    let v_19 = bitcast<vec2<u32>>(v_18);
    let v_20 = v_19.x;
    atomicStore(&(v_1), v_20);
    let v_21 = v_19.y;
    atomicStore(&(v_2), v_21);
    atomicStore(&(v_3), v_20);
    atomicStore(&(v_4), v_21);
  }
  var v_22 : vec4<f32>;
  {
    var v_23 : vec4<f32>;
    v_23 = v_18.xyxy;
    var v_24 : u32;
    v_24 = 1u;
    loop {
      var v_25 : vec4<f32>;
      let v_26 = v_24;
      let v_27 = v_23;
      let v_28 = bitcast<u32>(v_17);
      if ((v_26 < v_28)) {
        let v_29 = (v_26 + v_9);
        var v_30 : vec4<f32>;
        if ((v_29 >= v_28)) {
          let v_31 = v[v_29].tint_symbol;
          let v_32 = min(v_27.xy, v_31);
          var v_33 : vec4<f32> = v_27;
          v_33.x = v_32.x;
          var v_34 : vec4<f32> = v_33;
          v_34.y = v_32.y;
          let v_35 = v_34;
          let v_36 = max(v_35.zw, v_31);
          var v_37 : vec4<f32> = v_35;
          v_37.z = v_36.x;
          var v_38 : vec4<f32> = v_37;
          v_38.w = v_36.y;
          v_30 = v_38;
        } else {
          v_30 = v_27;
        }
        v_25 = v_30;
        continue;
      } else {
        v_22 = v_27;
        break;
      }

      continuing {
        v_23 = v_25;
        v_24 = (v_26 + 32u);
      }
    }
  }
  let v_39 = v_22;
  workgroupBarrier();
  _ = atomicMin(&(v_1), bitcast<u32>(v_39.x));
  _ = atomicMin(&(v_2), bitcast<u32>(v_39.y));
  _ = atomicMax(&(v_3), bitcast<u32>(v_39.z));
  _ = atomicMax(&(v_4), bitcast<u32>(v_39.w));
  workgroupBarrier();
  v_7.tint_symbol_9[0i] = vec4<f32>(bitcast<f32>(atomicLoad(&(v_1))), bitcast<f32>(atomicLoad(&(v_2))), bitcast<f32>(atomicLoad(&(v_3))), bitcast<f32>(atomicLoad(&(v_4))));
}
