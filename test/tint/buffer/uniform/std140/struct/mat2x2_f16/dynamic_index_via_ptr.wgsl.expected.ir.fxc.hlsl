SKIP: FAILED


enable f16;

struct Inner {
  @size(64)
  m : mat2x2<f16>,
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
  let l_a_i_a_i_m : mat2x2<f16> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec2<f16> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f16 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :40:24 error: binary: %26 is not in scope
    %25:u32 = add %24, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:65:5 note: %26 declared here
    %26:u32 = mul %53, 2u
    ^^^^^^^

:48:24 error: binary: %26 is not in scope
    %36:u32 = add %35, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:65:5 note: %26 declared here
    %26:u32 = mul %53, 2u
    ^^^^^^^

:65:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %26:u32 = mul %53, 2u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(4) {
  m:mat2x2<f16> @offset(0)
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
    %37:mat2x2<f16> = call %38, %36
    %l_a_i_a_i_m:mat2x2<f16> = let %37
    %40:u32 = add %10, %13
    %41:u32 = add %40, %16
    %42:u32 = div %41, 16u
    %43:ptr<uniform, vec4<u32>, read> = access %a, %42
    %44:u32 = mod %41, 16u
    %45:u32 = div %44, 4u
    %46:vec4<u32> = load %43
    %47:u32 = swizzle %46, z
    %48:u32 = swizzle %46, x
    %49:bool = eq %45, 2u
    %50:u32 = hlsl.ternary %48, %47, %49
    %51:vec2<f16> = bitcast %50
    %l_a_i_a_i_m_i:vec2<f16> = let %51
    %53:i32 = call %i
    %26:u32 = mul %53, 2u
    %54:u32 = add %10, %13
    %55:u32 = add %54, %16
    %56:u32 = add %55, %26
    %57:u32 = div %56, 16u
    %58:ptr<uniform, vec4<u32>, read> = access %a, %57
    %59:u32 = mod %56, 16u
    %60:u32 = div %59, 4u
    %61:u32 = load_vector_element %58, %60
    %62:u32 = mod %56, 4u
    %63:bool = eq %62, 0u
    %64:u32 = hlsl.ternary 16u, 0u, %63
    %65:u32 = shr %61, %64
    %66:f32 = hlsl.f16tof32 %65
    %67:f16 = convert %66
    %l_a_i_a_i_m_i_i:f16 = let %67
    ret
  }
}
%21 = func(%start_byte_offset:u32):Outer {
  $B4: {
    %70:array<Inner, 4> = call %28, %start_byte_offset
    %71:Outer = construct %70
    ret %71
  }
}
%28 = func(%start_byte_offset_1:u32):array<Inner, 4> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat2x2<f16>(vec2<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B6, b: $B7, c: $B8] {  # loop_1
      $B6: {  # initializer
        next_iteration 0u  # -> $B7
      }
      $B7 (%idx:u32): {  # body
        %75:bool = gte %idx, 4u
        if %75 [t: $B9] {  # if_1
          $B9: {  # true
            exit_loop  # loop_1
          }
        }
        %76:u32 = mul %idx, 64u
        %77:u32 = add %start_byte_offset_1, %76
        %78:ptr<function, Inner, read_write> = access %a_1, %idx
        %79:Inner = call %32, %77
        store %78, %79
        continue  # -> $B8
      }
      $B8: {  # continuing
        %80:u32 = add %idx, 1u
        next_iteration %80  # -> $B7
      }
    }
    %81:array<Inner, 4> = load %a_1
    ret %81
  }
}
%32 = func(%start_byte_offset_2:u32):Inner {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %83:mat2x2<f16> = call %38, %start_byte_offset_2
    %84:Inner = construct %83
    ret %84
  }
}
%38 = func(%start_byte_offset_3:u32):mat2x2<f16> {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %86:u32 = div %start_byte_offset_3, 16u
    %87:ptr<uniform, vec4<u32>, read> = access %a, %86
    %88:u32 = mod %start_byte_offset_3, 16u
    %89:u32 = div %88, 4u
    %90:vec4<u32> = load %87
    %91:u32 = swizzle %90, z
    %92:u32 = swizzle %90, x
    %93:bool = eq %89, 2u
    %94:u32 = hlsl.ternary %92, %91, %93
    %95:vec2<f16> = bitcast %94
    %96:u32 = add 4u, %start_byte_offset_3
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
    %107:mat2x2<f16> = construct %95, %106
    ret %107
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat2x2<f16>(vec2<f16>(0.0h))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %111:bool = gte %idx_1, 4u
        if %111 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %112:u32 = mul %idx_1, 256u
        %113:u32 = add %start_byte_offset_4, %112
        %114:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %115:Outer = call %21, %113
        store %114, %115
        continue  # -> $B15
      }
      $B15: {  # continuing
        %116:u32 = add %idx_1, 1u
        next_iteration %116  # -> $B14
      }
    }
    %117:array<Outer, 4> = load %a_2
    ret %117
  }
}

