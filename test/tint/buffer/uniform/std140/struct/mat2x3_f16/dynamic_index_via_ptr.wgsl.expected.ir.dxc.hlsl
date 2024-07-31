SKIP: FAILED


enable f16;

struct Inner {
  @size(64)
  m : mat2x3<f16>,
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
  let l_a_i_a_i_m : mat2x3<f16> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec3<f16> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f16 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:65:5 note: %23 declared here
    %23:u32 = mul %53, 2u
    ^^^^^^^

:43:24 error: binary: %23 is not in scope
    %29:u32 = add %28, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:65:5 note: %23 declared here
    %23:u32 = mul %53, 2u
    ^^^^^^^

:48:24 error: binary: %23 is not in scope
    %35:u32 = add %34, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:65:5 note: %23 declared here
    %23:u32 = mul %53, 2u
    ^^^^^^^

:53:24 error: binary: %23 is not in scope
    %41:u32 = add %40, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:65:5 note: %23 declared here
    %23:u32 = mul %53, 2u
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

    %23:u32 = mul %53, 2u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(8) {
  m:mat2x3<f16> @offset(0)
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
    %34:u32 = add %33, %16
    %35:u32 = add %34, %23
    %36:Inner = call %37, %35
    %l_a_i_a_i:Inner = let %36
    %39:u32 = add %10, %13
    %40:u32 = add %39, %16
    %41:u32 = add %40, %23
    %42:mat2x3<f16> = call %43, %41
    %l_a_i_a_i_m:mat2x3<f16> = let %42
    %45:u32 = add %10, %13
    %46:u32 = add %45, %16
    %47:u32 = div %46, 16u
    %48:ptr<uniform, vec4<u32>, read> = access %a, %47
    %49:vec4<u32> = load %48
    %50:vec4<f16> = bitcast %49
    %51:vec3<f16> = swizzle %50, xyz
    %l_a_i_a_i_m_i:vec3<f16> = let %51
    %53:i32 = call %i
    %23:u32 = mul %53, 2u
    %54:u32 = add %10, %13
    %55:u32 = add %54, %16
    %56:u32 = add %55, %23
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
%43 = func(%start_byte_offset:u32):mat2x3<f16> {
  $B4: {
    %70:u32 = div %start_byte_offset, 16u
    %71:ptr<uniform, vec4<u32>, read> = access %a, %70
    %72:vec4<u32> = load %71
    %73:vec4<f16> = bitcast %72
    %74:vec3<f16> = swizzle %73, xyz
    %75:u32 = add 8u, %start_byte_offset
    %76:u32 = div %75, 16u
    %77:ptr<uniform, vec4<u32>, read> = access %a, %76
    %78:vec4<u32> = load %77
    %79:vec4<f16> = bitcast %78
    %80:vec3<f16> = swizzle %79, xyz
    %81:mat2x3<f16> = construct %74, %80
    ret %81
  }
}
%37 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %83:mat2x3<f16> = call %43, %start_byte_offset_1
    %84:Inner = construct %83
    ret %84
  }
}
%31 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat2x3<f16>(vec3<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %88:bool = gte %idx, 4u
        if %88 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %89:u32 = mul %idx, 64u
        %90:u32 = add %start_byte_offset_2, %89
        %91:ptr<function, Inner, read_write> = access %a_1, %idx
        %92:Inner = call %37, %90
        store %91, %92
        continue  # -> $B9
      }
      $B9: {  # continuing
        %93:u32 = add %idx, 1u
        next_iteration %93  # -> $B8
      }
    }
    %94:array<Inner, 4> = load %a_1
    ret %94
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %96:array<Inner, 4> = call %31, %start_byte_offset_3
    %97:Outer = construct %96
    ret %97
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat2x3<f16>(vec3<f16>(0.0h))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %101:bool = gte %idx_1, 4u
        if %101 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %102:u32 = mul %idx_1, 256u
        %103:u32 = add %start_byte_offset_4, %102
        %104:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %105:Outer = call %25, %103
        store %104, %105
        continue  # -> $B15
      }
      $B15: {  # continuing
        %106:u32 = add %idx_1, 1u
        next_iteration %106  # -> $B14
      }
    }
    %107:array<Outer, 4> = load %a_2
    ret %107
  }
}

