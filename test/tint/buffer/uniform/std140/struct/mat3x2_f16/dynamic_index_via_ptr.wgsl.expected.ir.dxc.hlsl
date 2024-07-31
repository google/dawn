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

Failed to generate: :40:24 error: binary: %26 is not in scope
    %25:u32 = add %24, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:66:5 note: %26 declared here
    %26:u32 = mul %54, 2u
    ^^^^^^^

:48:24 error: binary: %26 is not in scope
    %36:u32 = add %35, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:66:5 note: %26 declared here
    %26:u32 = mul %54, 2u
    ^^^^^^^

:53:24 error: binary: %26 is not in scope
    %42:u32 = add %41, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:66:5 note: %26 declared here
    %26:u32 = mul %54, 2u
    ^^^^^^^

:66:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %26:u32 = mul %54, 2u
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
    %20:Outer = call %21, %10
    %l_a_i:Outer = let %20
    %23:u32 = add %10, %13
    %24:u32 = add %23, %16
    %25:u32 = add %24, %26
    %27:array<Inner, 4> = call %28, %25
    %l_a_i_a:array<Inner, 4> = let %27
    %30:u32 = add %10, %13
    %31:Inner = call %32, %30
    %l_a_i_a_i:Inner = let %31
    %34:u32 = add %10, %13
    %35:u32 = add %34, %16
    %36:u32 = add %35, %26
    %37:mat3x2<f16> = call %38, %36
    %l_a_i_a_i_m:mat3x2<f16> = let %37
    %40:u32 = add %10, %13
    %41:u32 = add %40, %16
    %42:u32 = add %41, %26
    %43:u32 = div %42, 16u
    %44:ptr<uniform, vec4<u32>, read> = access %a, %43
    %45:u32 = mod %42, 16u
    %46:u32 = div %45, 4u
    %47:vec4<u32> = load %44
    %48:u32 = swizzle %47, z
    %49:u32 = swizzle %47, x
    %50:bool = eq %46, 2u
    %51:u32 = hlsl.ternary %49, %48, %50
    %52:vec2<f16> = bitcast %51
    %l_a_i_a_i_m_i:vec2<f16> = let %52
    %54:i32 = call %i
    %26:u32 = mul %54, 2u
    %55:u32 = add %10, %13
    %56:u32 = add %55, %16
    %57:u32 = add %56, %26
    %58:u32 = div %57, 16u
    %59:ptr<uniform, vec4<u32>, read> = access %a, %58
    %60:u32 = mod %57, 16u
    %61:u32 = div %60, 4u
    %62:u32 = load_vector_element %59, %61
    %63:u32 = mod %57, 4u
    %64:bool = eq %63, 0u
    %65:u32 = hlsl.ternary 16u, 0u, %64
    %66:u32 = shr %62, %65
    %67:f32 = hlsl.f16tof32 %66
    %68:f16 = convert %67
    %l_a_i_a_i_m_i_i:f16 = let %68
    ret
  }
}
%21 = func(%start_byte_offset:u32):Outer {
  $B4: {
    %71:array<Inner, 4> = call %28, %start_byte_offset
    %72:Outer = construct %71
    ret %72
  }
}
%28 = func(%start_byte_offset_1:u32):array<Inner, 4> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat3x2<f16>(vec2<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B6, b: $B7, c: $B8] {  # loop_1
      $B6: {  # initializer
        next_iteration 0u  # -> $B7
      }
      $B7 (%idx:u32): {  # body
        %76:bool = gte %idx, 4u
        if %76 [t: $B9] {  # if_1
          $B9: {  # true
            exit_loop  # loop_1
          }
        }
        %77:u32 = mul %idx, 64u
        %78:u32 = add %start_byte_offset_1, %77
        %79:ptr<function, Inner, read_write> = access %a_1, %idx
        %80:Inner = call %32, %78
        store %79, %80
        continue  # -> $B8
      }
      $B8: {  # continuing
        %81:u32 = add %idx, 1u
        next_iteration %81  # -> $B7
      }
    }
    %82:array<Inner, 4> = load %a_1
    ret %82
  }
}
%32 = func(%start_byte_offset_2:u32):Inner {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %84:mat3x2<f16> = call %38, %start_byte_offset_2
    %85:Inner = construct %84
    ret %85
  }
}
%38 = func(%start_byte_offset_3:u32):mat3x2<f16> {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %87:u32 = div %start_byte_offset_3, 16u
    %88:ptr<uniform, vec4<u32>, read> = access %a, %87
    %89:u32 = mod %start_byte_offset_3, 16u
    %90:u32 = div %89, 4u
    %91:vec4<u32> = load %88
    %92:u32 = swizzle %91, z
    %93:u32 = swizzle %91, x
    %94:bool = eq %90, 2u
    %95:u32 = hlsl.ternary %93, %92, %94
    %96:vec2<f16> = bitcast %95
    %97:u32 = add 4u, %start_byte_offset_3
    %98:u32 = div %97, 16u
    %99:ptr<uniform, vec4<u32>, read> = access %a, %98
    %100:u32 = mod %97, 16u
    %101:u32 = div %100, 4u
    %102:vec4<u32> = load %99
    %103:u32 = swizzle %102, z
    %104:u32 = swizzle %102, x
    %105:bool = eq %101, 2u
    %106:u32 = hlsl.ternary %104, %103, %105
    %107:vec2<f16> = bitcast %106
    %108:u32 = add 8u, %start_byte_offset_3
    %109:u32 = div %108, 16u
    %110:ptr<uniform, vec4<u32>, read> = access %a, %109
    %111:u32 = mod %108, 16u
    %112:u32 = div %111, 4u
    %113:vec4<u32> = load %110
    %114:u32 = swizzle %113, z
    %115:u32 = swizzle %113, x
    %116:bool = eq %112, 2u
    %117:u32 = hlsl.ternary %115, %114, %116
    %118:vec2<f16> = bitcast %117
    %119:mat3x2<f16> = construct %96, %107, %118
    ret %119
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
        %123:bool = gte %idx_1, 4u
        if %123 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %124:u32 = mul %idx_1, 256u
        %125:u32 = add %start_byte_offset_4, %124
        %126:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %127:Outer = call %21, %125
        store %126, %127
        continue  # -> $B15
      }
      $B15: {  # continuing
        %128:u32 = add %idx_1, 1u
        next_iteration %128  # -> $B14
      }
    }
    %129:array<Outer, 4> = load %a_2
    ret %129
  }
}

