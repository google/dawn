# Multi Planar Format NV12A (Experimental!)

The `multi-planar-format-nv12a` feature allows rendering NV12 format with an extra alpha plane. A typical usage scenario of this feature is HEVC with Alpha video decoding feature on Chrome for Mac. macOS will directly decode the buffer into such format without extra YUVA to RGBA conversion, which turns out has better power efficiency.

Notes:
  - The format is only available on macOS >= 10.15.
