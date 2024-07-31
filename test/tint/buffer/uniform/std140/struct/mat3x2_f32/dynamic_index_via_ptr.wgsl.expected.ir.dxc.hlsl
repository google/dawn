SKIP: FAILED


struct Inner {
  @size(64)
  m : mat3x2<f32>,
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
  let l_a_i_a_i_m : mat3x2<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec2<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 4u
    ^^^^^^^

:45:24 error: binary: %23 is not in scope
    %32:u32 = add %31, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 4u
    ^^^^^^^

:50:24 error: binary: %23 is not in scope
    %38:u32 = add %37, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 4u
    ^^^^^^^

:55:24 error: binary: %23 is not in scope
    %44:u32 = add %43, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:68:5 note: %23 declared here
    %23:u32 = mul %56, 4u
    ^^^^^^^

:68:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %23:u32 = mul %56, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(8) {
  m:mat3x2<f32> @offset(0)
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
    %27:array<Inner, 4> = call %28, %10
    %l_a_i_a:array<Inner, 4> = let %27
    %30:u32 = add %10, %13
    %31:u32 = add %30, %16
    %32:u32 = add %31, %23
    %33:Inner = call %34, %32
    %l_a_i_a_i:Inner = let %33
    %36:u32 = add %10, %13
    %37:u32 = add %36, %16
    %38:u32 = add %37, %23
    %39:mat3x2<f32> = call %40, %38
    %l_a_i_a_i_m:mat3x2<f32> = let %39
    %42:u32 = add %10, %13
    %43:u32 = add %42, %16
    %44:u32 = add %43, %23
    %45:u32 = div %44, 16u
    %46:ptr<uniform, vec4<u32>, read> = access %a, %45
    %47:u32 = mod %44, 16u
    %48:u32 = div %47, 4u
    %49:vec4<u32> = load %46
    %50:vec2<u32> = swizzle %49, zw
    %51:vec2<u32> = swizzle %49, xy
    %52:bool = eq %48, 2u
    %53:vec2<u32> = hlsl.ternary %51, %50, %52
    %54:vec2<f32> = bitcast %53
    %l_a_i_a_i_m_i:vec2<f32> = let %54
    %56:i32 = call %i
    %23:u32 = mul %56, 4u
    %57:u32 = add %10, %13
    %58:u32 = add %57, %16
    %59:u32 = add %58, %23
    %60:u32 = div %59, 16u
    %61:ptr<uniform, vec4<u32>, read> = access %a, %60
    %62:u32 = mod %59, 16u
    %63:u32 = div %62, 4u
    %64:u32 = load_vector_element %61, %63
    %65:f32 = bitcast %64
    %l_a_i_a_i_m_i_i:f32 = let %65
    ret
  }
}
%18 = func(%start_byte_offset:u32):array<Outer, 4> {
  $B4: {
    %a_1:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat3x2<f32>(vec2<f32>(0.0f))))))  # %a_1: 'a'
    loop [i: $B5, b: $B6, c: $B7] {  # loop_1
      $B5: {  # initializer
        next_iteration 0u  # -> $B6
      }
      $B6 (%idx:u32): {  # body
        %70:bool = gte %idx, 4u
        if %70 [t: $B8] {  # if_1
          $B8: {  # true
            exit_loop  # loop_1
          }
        }
        %71:u32 = mul %idx, 256u
        %72:u32 = add %start_byte_offset, %71
        %73:ptr<function, Outer, read_write> = access %a_1, %idx
        %74:Outer = call %25, %72
        store %73, %74
        continue  # -> $B7
      }
      $B7: {  # continuing
        %75:u32 = add %idx, 1u
        next_iteration %75  # -> $B6
      }
    }
    %76:array<Outer, 4> = load %a_1
    ret %76
  }
}
%25 = func(%start_byte_offset_1:u32):Outer {  # %start_byte_offset_1: 'start_byte_offset'
  $B9: {
    %78:array<Inner, 4> = call %28, %start_byte_offset_1
    %79:Outer = construct %78
    ret %79
  }
}
%28 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %a_2:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat3x2<f32>(vec2<f32>(0.0f))))  # %a_2: 'a'
    loop [i: $B11, b: $B12, c: $B13] {  # loop_2
      $B11: {  # initializer
        next_iteration 0u  # -> $B12
      }
      $B12 (%idx_1:u32): {  # body
        %83:bool = gte %idx_1, 4u
        if %83 [t: $B14] {  # if_2
          $B14: {  # true
            exit_loop  # loop_2
          }
        }
        %84:u32 = mul %idx_1, 64u
        %85:u32 = add %start_byte_offset_2, %84
        %86:ptr<function, Inner, read_write> = access %a_2, %idx_1
        %87:Inner = call %34, %85
        store %86, %87
        continue  # -> $B13
      }
      $B13: {  # continuing
        %88:u32 = add %idx_1, 1u
        next_iteration %88  # -> $B12
      }
    }
    %89:array<Inner, 4> = load %a_2
    ret %89
  }
}
%34 = func(%start_byte_offset_3:u32):Inner {  # %start_byte_offset_3: 'start_byte_offset'
  $B15: {
    %91:mat3x2<f32> = call %40, %start_byte_offset_3
    %92:Inner = construct %91
    ret %92
  }
}
%40 = func(%start_byte_offset_4:u32):mat3x2<f32> {  # %start_byte_offset_4: 'start_byte_offset'
  $B16: {
    %94:u32 = div %start_byte_offset_4, 16u
    %95:ptr<uniform, vec4<u32>, read> = access %a, %94
    %96:u32 = mod %start_byte_offset_4, 16u
    %97:u32 = div %96, 4u
    %98:vec4<u32> = load %95
    %99:vec2<u32> = swizzle %98, zw
    %100:vec2<u32> = swizzle %98, xy
    %101:bool = eq %97, 2u
    %102:vec2<u32> = hlsl.ternary %100, %99, %101
    %103:vec2<f32> = bitcast %102
    %104:u32 = add 8u, %start_byte_offset_4
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
    %115:u32 = add 16u, %start_byte_offset_4
    %116:u32 = div %115, 16u
    %117:ptr<uniform, vec4<u32>, read> = access %a, %116
    %118:u32 = mod %115, 16u
    %119:u32 = div %118, 4u
    %120:vec4<u32> = load %117
    %121:vec2<u32> = swizzle %120, zw
    %122:vec2<u32> = swizzle %120, xy
    %123:bool = eq %119, 2u
    %124:vec2<u32> = hlsl.ternary %122, %121, %123
    %125:vec2<f32> = bitcast %124
    %126:mat3x2<f32> = construct %103, %114, %125
    ret %126
  }
}

