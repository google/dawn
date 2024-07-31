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

Failed to generate: :42:24 error: binary: %29 is not in scope
    %28:u32 = add %27, %29
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:63:5 note: %29 declared here
    %29:u32 = mul %51, 2u
    ^^^^^^^

:50:24 error: binary: %29 is not in scope
    %39:u32 = add %38, %29
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:63:5 note: %29 declared here
    %29:u32 = mul %51, 2u
    ^^^^^^^

:63:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %29:u32 = mul %51, 2u
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
    %23:array<Inner, 4> = call %24, %10
    %l_a_i_a:array<Inner, 4> = let %23
    %26:u32 = add %10, %13
    %27:u32 = add %26, %16
    %28:u32 = add %27, %29
    %30:Inner = call %31, %28
    %l_a_i_a_i:Inner = let %30
    %33:u32 = add %10, %13
    %34:mat2x2<f16> = call %35, %33
    %l_a_i_a_i_m:mat2x2<f16> = let %34
    %37:u32 = add %10, %13
    %38:u32 = add %37, %16
    %39:u32 = add %38, %29
    %40:u32 = div %39, 16u
    %41:ptr<uniform, vec4<u32>, read> = access %a, %40
    %42:u32 = mod %39, 16u
    %43:u32 = div %42, 4u
    %44:vec4<u32> = load %41
    %45:u32 = swizzle %44, z
    %46:u32 = swizzle %44, x
    %47:bool = eq %43, 2u
    %48:u32 = hlsl.ternary %46, %45, %47
    %49:vec2<f16> = bitcast %48
    %l_a_i_a_i_m_i:vec2<f16> = let %49
    %51:i32 = call %i
    %29:u32 = mul %51, 2u
    %52:u32 = add %10, %13
    %53:u32 = add %52, %16
    %54:u32 = add %53, %29
    %55:u32 = div %54, 16u
    %56:ptr<uniform, vec4<u32>, read> = access %a, %55
    %57:u32 = mod %54, 16u
    %58:u32 = div %57, 4u
    %59:u32 = load_vector_element %56, %58
    %60:u32 = mod %54, 4u
    %61:bool = eq %60, 0u
    %62:u32 = hlsl.ternary 16u, 0u, %61
    %63:u32 = shr %59, %62
    %64:f32 = hlsl.f16tof32 %63
    %65:f16 = convert %64
    %l_a_i_a_i_m_i_i:f16 = let %65
    ret
  }
}
%18 = func(%start_byte_offset:u32):array<Outer, 4> {
  $B4: {
    %a_1:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat2x2<f16>(vec2<f16>(0.0h))))))  # %a_1: 'a'
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
        %74:Outer = call %21, %72
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
%21 = func(%start_byte_offset_1:u32):Outer {  # %start_byte_offset_1: 'start_byte_offset'
  $B9: {
    %78:array<Inner, 4> = call %24, %start_byte_offset_1
    %79:Outer = construct %78
    ret %79
  }
}
%24 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %a_2:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat2x2<f16>(vec2<f16>(0.0h))))  # %a_2: 'a'
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
        %87:Inner = call %31, %85
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
%31 = func(%start_byte_offset_3:u32):Inner {  # %start_byte_offset_3: 'start_byte_offset'
  $B15: {
    %91:mat2x2<f16> = call %35, %start_byte_offset_3
    %92:Inner = construct %91
    ret %92
  }
}
%35 = func(%start_byte_offset_4:u32):mat2x2<f16> {  # %start_byte_offset_4: 'start_byte_offset'
  $B16: {
    %94:u32 = div %start_byte_offset_4, 16u
    %95:ptr<uniform, vec4<u32>, read> = access %a, %94
    %96:u32 = mod %start_byte_offset_4, 16u
    %97:u32 = div %96, 4u
    %98:vec4<u32> = load %95
    %99:u32 = swizzle %98, z
    %100:u32 = swizzle %98, x
    %101:bool = eq %97, 2u
    %102:u32 = hlsl.ternary %100, %99, %101
    %103:vec2<f16> = bitcast %102
    %104:u32 = add 4u, %start_byte_offset_4
    %105:u32 = div %104, 16u
    %106:ptr<uniform, vec4<u32>, read> = access %a, %105
    %107:u32 = mod %104, 16u
    %108:u32 = div %107, 4u
    %109:vec4<u32> = load %106
    %110:u32 = swizzle %109, z
    %111:u32 = swizzle %109, x
    %112:bool = eq %108, 2u
    %113:u32 = hlsl.ternary %111, %110, %112
    %114:vec2<f16> = bitcast %113
    %115:mat2x2<f16> = construct %103, %114
    ret %115
  }
}

