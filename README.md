# vkperf
Vulkan Performance Measurement Tool

vkperf tests various performance characteristics of Vulkan devices.
It is aimed on developer needs. It provides them with performance characteristics of their devices,
allowing them to make better decisions during the design of their rendering algorithms and the design of Vulkan-related code in general.

## Usage
<pre>
vkperf [deviceNameFilter] [deviceIndex] [options]
    --long - perform long test; testing time is extended to 20 second
             from the default of 2 seconds
    --minimal - perform minimal test; for debugging purposes,
                number of triangles used for testing is lagerly
                reduced, making measurements possibly very imprecise
    --sparse-none - no sparse mode is used during the main test;
                    this is the default
    --sparse-binding - sparse binding mode is used during the main test
    --sparse-residency - sparse residency mode is used during the main test
    --sparse-residency-aliased - sparse residency aliased mode is used
                                 during the main test
    --debug or -d - print additional debug info
    --help or -h - prints the usage information
</pre>

## Tests performed
* Triangle throughput
  * Vertex shader throughput
  * Geometry shader throughput
  * Instancing performance
  * Draw command throughput
  * Indirect command throughput
  * Attribute input overhead
  * Buffer read overhead
  * Transformation matrices overhead
  * Phong light model overhead
  * Triangle strip performance
  * Indexed rendering performance
  * Primitive restart performance
* Fragment throughput
  * Fullscreen quad fragment throughput
  * Interpolators overhead
  * Textured phong performance
* Transfer throughput
  * Various block sizes transfer performance
* Memory allocation performance
  * Standard memory and Sparse memory allocation performance
  * Buffer creation time for various buffer sizes
  * DeviceMemory creation time for various memory sizes
  * Bind time for various Buffer and DeviceMemory sizes

## License
Public domain

## Author
PC John (Jan Pečiva, peciva _at fit.vut.cz)

