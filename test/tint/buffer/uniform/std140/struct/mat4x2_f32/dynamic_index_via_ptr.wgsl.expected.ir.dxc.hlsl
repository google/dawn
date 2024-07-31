SKIP: FAILED


struct Inner {
  @size(64)
  m : mat4x2<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat4x2<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec2<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:69:5 note: %23 declared here
    %23:u32 = mul %57, 4u
    ^^^^^^^

:43:24 error: binary: %23 is not in scope
    %29:u32 = add %28, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:69:5 note: %23 declared here
    %23:u32 = mul %57, 4u
    ^^^^^^^

:51:24 error: binary: %23 is not in scope
    %39:u32 = add %38, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:69:5 note: %23 declared here
    %23:u32 = mul %57, 4u
    ^^^^^^^

:56:24 error: binary: %23 is not in scope
    %45:u32 = add %44, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:69:5 note: %23 declared here
    %23:u32 = mul %57, 4u
    ^^^^^^^

:69:5 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

    %23:u32 = mul %57, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(8) {
  m:mat4x2<f32> @offset(0)
}

Outer = struct @align(8) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 8u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:u32 = add %10, %13
    %21:u32 = add %20, %16
    %22:u32 = add %21, %23
    %24:Outer = call %25, %22
    %l_a_i:Outer = let %24
    %27:u32 = add %10, %13
    %28:u32 = add %27, %16
    %29:u32 = add %28, %23
    %30:array<Inner, 4> = call %31, %29
    %l_a_i_a:array<Inner, 4> = let %30
    %33:u32 = add %10, %13
    %34:Inner = call %35, %33
    %l_a_i_a_i:Inner = let %34
    %37:u32 = add %10, %13
    %38:u32 = add %37, %16
    %39:u32 = add %38, %23
    %40:mat4x2<f32> = call %41, %39
    %l_a_i_a_i_m:mat4x2<f32> = let %40
    %43:u32 = add %10, %13
    %44:u32 = add %43, %16
    %45:u32 = add %44, %23
    %46:u32 = div %45, 16u
    %47:ptr<uniform, vec4<u32>, read> = access %a, %46
    %48:u32 = mod %45, 16u
    %49:u32 = div %48, 4u
    %50:vec4<u32> = load %47
    %51:vec2<u32> = swizzle %50, zw
    %52:vec2<u32> = swizzle %50, xy
    %53:bool = eq %49, 2u
    %54:vec2<u32> = hlsl.ternary %52, %51, %53
    %55:vec2<f32> = bitcast %54
    %l_a_i_a_i_m_i:vec2<f32> = let %55
    %57:i32 = call %i
    %23:u32 = mul %57, 4u
    %58:u32 = add %10, %13
    %59:u32 = add %58, %16
    %60:u32 = add %59, %23
    %61:u32 = div %60, 16u
    %62:ptr<uniform, vec4<u32>, read> = access %a, %61
    %63:u32 = mod %60, 16u
    %64:u32 = div %63, 4u
    %65:u32 = load_vector_element %62, %64
    %66:f32 = bitcast %65
    %l_a_i_a_i_m_i_i:f32 = let %66
    ret
  }
}
%35 = func(%start_byte_offset:u32):Inner {
  $B4: {
    %69:mat4x2<f32> = call %41, %start_byte_offset
    %70:Inner = construct %69
    ret %70
  }
}
%41 = func(%start_byte_offset_1:u32):mat4x2<f32> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %72:u32 = div %start_byte_offset_1, 16u
    %73:ptr<uniform, vec4<u32>, read> = access %a, %72
    %74:u32 = mod %start_byte_offset_1, 16u
    %75:u32 = div %74, 4u
    %76:vec4<u32> = load %73
    %77:vec2<u32> = swizzle %76, zw
    %78:vec2<u32> = swizzle %76, xy
    %79:bool = eq %75, 2u
    %80:vec2<u32> = hlsl.ternary %78, %77, %79
    %81:vec2<f32> = bitcast %80
    %82:u32 = add 8u, %start_byte_offset_1
    %83:u32 = div %82, 16u
    %84:ptr<uniform, vec4<u32>, read> = access %a, %83
    %85:u32 = mod %82, 16u
    %86:u32 = div %85, 4u
    %87:vec4<u32> = load %84
    %88:vec2<u32> = swizzle %87, zw
    %89:vec2<u32> = swizzle %87, xy
    %90:bool = eq %86, 2u
    %91:vec2<u32> = hlsl.ternary %89, %88, %90
    %92:vec2<f32> = bitcast %91
    %93:u32 = add 16u, %start_byte_offset_1
    %94:u32 = div %93, 16u
    %95:ptr<uniform, vec4<u32>, read> = access %a, %94
    %96:u32 = mod %93, 16u
    %97:u32 = div %96, 4u
    %98:vec4<u32> = load %95
    %99:vec2<u32> = swizzle %98, zw
    %100:vec2<u32> = swizzle %98, xy
    %101:bool = eq %97, 2u
    %102:vec2<u32> = hlsl.ternary %100, %99, %101
    %103:vec2<f32> = bitcast %102
    %104:u32 = add 24u, %start_byte_offset_1
    %105:u32 = div %104, 16u
    %106:ptr<uniform, vec4<u32>, read> = access %a, %105
    %107:u32 = mod %104, 16u
    %108:u32 = div %107, 4u
    %109:vec4<u32> = load %106
    %110:vec2<u32> = swizzle %109, zw
    %111:vec2<u32> = swizzle %109, xy
    %112:bool = eq %108, 2u
    %113:vec2<u32> = hlsl.ternary %111, %110, %112
    %114:vec2<f32> = bitcast %113
    %115:mat4x2<f32> = construct %81, %92, %103, %114
    ret %115
  }
}
%31 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x2<f32>(vec2<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %119:bool = gte %idx, 4u
        if %119 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %120:u32 = mul %idx, 64u
        %121:u32 = add %start_byte_offset_2, %120
        %122:ptr<function, Inner, read_write> = access %a_1, %idx
        %123:Inner = call %35, %121
        store %122, %123
        continue  # -> $B9
      }
      $B9: {  # continuing
        %124:u32 = add %idx, 1u
        next_iteration %124  # -> $B8
      }
    }
    %125:array<Inner, 4> = load %a_1
    ret %125
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %127:array<Inner, 4> = call %31, %start_byte_offset_3
    %128:Outer = construct %127
    ret %128
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat4x2<f32>(vec2<f32>(0.0f))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %132:bool = gte %idx_1, 4u
        if %132 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %133:u32 = mul %idx_1, 256u
        %134:u32 = add %start_byte_offset_4, %133
        %135:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %136:Outer = call %25, %134
        store %135, %136
        continue  # -> $B15
      }
      $B15: {  # continuing
        %137:u32 = add %idx_1, 1u
        next_iteration %137  # -> $B14
      }
    }
    %138:array<Outer, 4> = load %a_2
    ret %138
  }
}

