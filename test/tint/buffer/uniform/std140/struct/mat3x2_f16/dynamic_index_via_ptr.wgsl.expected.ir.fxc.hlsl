SKIP: FAILED


enable f16;

struct Inner {
  @size(64)
  m : mat3x2<f16>,
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
  let l_a_i_a_i_m : mat3x2<f16> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec2<f16> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f16 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:70:5 note: %23 declared here
    %23:u32 = mul %58, 2u
    ^^^^^^^

:43:24 error: binary: %23 is not in scope
    %29:u32 = add %28, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:70:5 note: %23 declared here
    %23:u32 = mul %58, 2u
    ^^^^^^^

:48:24 error: binary: %23 is not in scope
    %35:u32 = add %34, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:70:5 note: %23 declared here
    %23:u32 = mul %58, 2u
    ^^^^^^^

:53:24 error: binary: %23 is not in scope
    %41:u32 = add %40, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:70:5 note: %23 declared here
    %23:u32 = mul %58, 2u
    ^^^^^^^

:70:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %23:u32 = mul %58, 2u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(4) {
  m:mat3x2<f16> @offset(0)
}

Outer = struct @align(4) {
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
    %16:u32 = mul 4u, %15
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
    %34:u32 = add %33, %16
    %35:u32 = add %34, %23
    %36:Inner = call %37, %35
    %l_a_i_a_i:Inner = let %36
    %39:u32 = add %10, %13
    %40:u32 = add %39, %16
    %41:u32 = add %40, %23
    %42:mat3x2<f16> = call %43, %41
    %l_a_i_a_i_m:mat3x2<f16> = let %42
    %45:u32 = add %10, %13
    %46:u32 = add %45, %16
    %47:u32 = div %46, 16u
    %48:ptr<uniform, vec4<u32>, read> = access %a, %47
    %49:u32 = mod %46, 16u
    %50:u32 = div %49, 4u
    %51:vec4<u32> = load %48
    %52:u32 = swizzle %51, z
    %53:u32 = swizzle %51, x
    %54:bool = eq %50, 2u
    %55:u32 = hlsl.ternary %53, %52, %54
    %56:vec2<f16> = bitcast %55
    %l_a_i_a_i_m_i:vec2<f16> = let %56
    %58:i32 = call %i
    %23:u32 = mul %58, 2u
    %59:u32 = add %10, %13
    %60:u32 = add %59, %16
    %61:u32 = add %60, %23
    %62:u32 = div %61, 16u
    %63:ptr<uniform, vec4<u32>, read> = access %a, %62
    %64:u32 = mod %61, 16u
    %65:u32 = div %64, 4u
    %66:u32 = load_vector_element %63, %65
    %67:u32 = mod %61, 4u
    %68:bool = eq %67, 0u
    %69:u32 = hlsl.ternary 16u, 0u, %68
    %70:u32 = shr %66, %69
    %71:f32 = hlsl.f16tof32 %70
    %72:f16 = convert %71
    %l_a_i_a_i_m_i_i:f16 = let %72
    ret
  }
}
%43 = func(%start_byte_offset:u32):mat3x2<f16> {
  $B4: {
    %75:u32 = div %start_byte_offset, 16u
    %76:ptr<uniform, vec4<u32>, read> = access %a, %75
    %77:u32 = mod %start_byte_offset, 16u
    %78:u32 = div %77, 4u
    %79:vec4<u32> = load %76
    %80:u32 = swizzle %79, z
    %81:u32 = swizzle %79, x
    %82:bool = eq %78, 2u
    %83:u32 = hlsl.ternary %81, %80, %82
    %84:vec2<f16> = bitcast %83
    %85:u32 = add 4u, %start_byte_offset
    %86:u32 = div %85, 16u
    %87:ptr<uniform, vec4<u32>, read> = access %a, %86
    %88:u32 = mod %85, 16u
    %89:u32 = div %88, 4u
    %90:vec4<u32> = load %87
    %91:u32 = swizzle %90, z
    %92:u32 = swizzle %90, x
    %93:bool = eq %89, 2u
    %94:u32 = hlsl.ternary %92, %91, %93
    %95:vec2<f16> = bitcast %94
    %96:u32 = add 8u, %start_byte_offset
    %97:u32 = div %96, 16u
    %98:ptr<uniform, vec4<u32>, read> = access %a, %97
    %99:u32 = mod %96, 16u
    %100:u32 = div %99, 4u
    %101:vec4<u32> = load %98
    %102:u32 = swizzle %101, z
    %103:u32 = swizzle %101, x
    %104:bool = eq %100, 2u
    %105:u32 = hlsl.ternary %103, %102, %104
    %106:vec2<f16> = bitcast %105
    %107:mat3x2<f16> = construct %84, %95, %106
    ret %107
  }
}
%37 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %109:mat3x2<f16> = call %43, %start_byte_offset_1
    %110:Inner = construct %109
    ret %110
  }
}
%31 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat3x2<f16>(vec2<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %114:bool = gte %idx, 4u
        if %114 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %115:u32 = mul %idx, 64u
        %116:u32 = add %start_byte_offset_2, %115
        %117:ptr<function, Inner, read_write> = access %a_1, %idx
        %118:Inner = call %37, %116
        store %117, %118
        continue  # -> $B9
      }
      $B9: {  # continuing
        %119:u32 = add %idx, 1u
        next_iteration %119  # -> $B8
      }
    }
    %120:array<Inner, 4> = load %a_1
    ret %120
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %122:array<Inner, 4> = call %31, %start_byte_offset_3
    %123:Outer = construct %122
    ret %123
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat3x2<f16>(vec2<f16>(0.0h))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %127:bool = gte %idx_1, 4u
        if %127 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %128:u32 = mul %idx_1, 256u
        %129:u32 = add %start_byte_offset_4, %128
        %130:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %131:Outer = call %25, %129
        store %130, %131
        continue  # -> $B15
      }
      $B15: {  # continuing
        %132:u32 = add %idx_1, 1u
        next_iteration %132  # -> $B14
      }
    }
    %133:array<Outer, 4> = load %a_2
    ret %133
  }
}

