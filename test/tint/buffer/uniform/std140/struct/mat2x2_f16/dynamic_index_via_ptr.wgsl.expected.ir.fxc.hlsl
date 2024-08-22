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

Failed to generate: :60:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %48:u32 = mul %47, 2u
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
    %27:Inner = call %28, %26
    %l_a_i_a_i:Inner = let %27
    %30:u32 = add %10, %13
    %31:mat2x2<f16> = call %32, %30
    %l_a_i_a_i_m:mat2x2<f16> = let %31
    %34:u32 = add %10, %13
    %35:u32 = add %34, %16
    %36:u32 = div %35, 16u
    %37:ptr<uniform, vec4<u32>, read> = access %a, %36
    %38:u32 = mod %35, 16u
    %39:u32 = div %38, 4u
    %40:vec4<u32> = load %37
    %41:u32 = swizzle %40, z
    %42:u32 = swizzle %40, x
    %43:bool = eq %39, 2u
    %44:u32 = hlsl.ternary %42, %41, %43
    %45:vec2<f16> = bitcast %44
    %l_a_i_a_i_m_i:vec2<f16> = let %45
    %47:i32 = call %i
    %48:u32 = mul %47, 2u
    %49:u32 = add %10, %13
    %50:u32 = add %49, %16
    %51:u32 = add %50, %48
    %52:u32 = div %51, 16u
    %53:ptr<uniform, vec4<u32>, read> = access %a, %52
    %54:u32 = mod %51, 16u
    %55:u32 = div %54, 4u
    %56:u32 = load_vector_element %53, %55
    %57:u32 = mod %51, 4u
    %58:bool = eq %57, 0u
    %59:u32 = hlsl.ternary 16u, 0u, %58
    %60:u32 = shr %56, %59
    %61:f32 = hlsl.f16tof32 %60
    %62:f16 = convert %61
    %l_a_i_a_i_m_i_i:f16 = let %62
    ret
  }
}
%28 = func(%start_byte_offset:u32):Inner {
  $B4: {
    %65:mat2x2<f16> = call %32, %start_byte_offset
    %66:Inner = construct %65
    ret %66
  }
}
%32 = func(%start_byte_offset_1:u32):mat2x2<f16> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %68:u32 = div %start_byte_offset_1, 16u
    %69:ptr<uniform, vec4<u32>, read> = access %a, %68
    %70:u32 = mod %start_byte_offset_1, 16u
    %71:u32 = div %70, 4u
    %72:vec4<u32> = load %69
    %73:u32 = swizzle %72, z
    %74:u32 = swizzle %72, x
    %75:bool = eq %71, 2u
    %76:u32 = hlsl.ternary %74, %73, %75
    %77:vec2<f16> = bitcast %76
    %78:u32 = add 4u, %start_byte_offset_1
    %79:u32 = div %78, 16u
    %80:ptr<uniform, vec4<u32>, read> = access %a, %79
    %81:u32 = mod %78, 16u
    %82:u32 = div %81, 4u
    %83:vec4<u32> = load %80
    %84:u32 = swizzle %83, z
    %85:u32 = swizzle %83, x
    %86:bool = eq %82, 2u
    %87:u32 = hlsl.ternary %85, %84, %86
    %88:vec2<f16> = bitcast %87
    %89:mat2x2<f16> = construct %77, %88
    ret %89
  }
}
%24 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat2x2<f16>(vec2<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %93:bool = gte %idx, 4u
        if %93 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %94:u32 = mul %idx, 64u
        %95:u32 = add %start_byte_offset_2, %94
        %96:ptr<function, Inner, read_write> = access %a_1, %idx
        %97:Inner = call %28, %95
        store %96, %97
        continue  # -> $B9
      }
      $B9: {  # continuing
        %98:u32 = add %idx, 1u
        next_iteration %98  # -> $B8
      }
    }
    %99:array<Inner, 4> = load %a_1
    ret %99
  }
}
%21 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %101:array<Inner, 4> = call %24, %start_byte_offset_3
    %102:Outer = construct %101
    ret %102
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
        %106:bool = gte %idx_1, 4u
        if %106 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %107:u32 = mul %idx_1, 256u
        %108:u32 = add %start_byte_offset_4, %107
        %109:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %110:Outer = call %21, %108
        store %109, %110
        continue  # -> $B15
      }
      $B15: {  # continuing
        %111:u32 = add %idx_1, 1u
        next_iteration %111  # -> $B14
      }
    }
    %112:array<Outer, 4> = load %a_2
    ret %112
  }
}

