# vkperf tests

Vulkan Performance Measurement Tool perform various tests to measure performance characteristics of particular Vulkan device.
The tests are described in detail in this file.

The list of tests follows:
1. [VS max throughput](#vs-max-throughput)
2. [VS VertexIndex and InstanceIndex forming output](#vs-vertexindex-and-instanceindex-forming-output)
3. [GS max throughput when no output is produced](#gs-max-throughput-when-no-output-is-produced)
4. [GS max throughput when single constant triangle is produced](#gs-max-throughput-when-single-constant-triangle-is-produced)
5. [GS max throughput when two constant triangles are produced](#gs-max-throughput-when-two-constant-triangles-are-produced)
6. [Instancing throughput of vkCmdDraw()](#instancing-throughput-of-vkcmddraw)
7. [Instancing throughput of vkCmdDrawIndirect()](#instancing-throughput-of-vkcmddrawindirect)

## VS max throughput

Vertex shader maximum throughput test uses simple shader without any attributes, buffers, descriptor sets, push constants or specialization constants.
It just outputs single position for all vertices:
<pre>
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
</pre>
Rendering is performed using single draw call for the whole scene:
<pre>
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
</pre>

## VS VertexIndex and InstanceIndex forming output

Vertex shader maximum throughput test uses simple shader that utilizes gl_VertexIndex and gl_InstanceIndex input variables.
No attributes, buffers, descriptor sets, push constants or specialization constants are used.
<pre>
void main() {
	gl_Position = vec4(0, 0, float(gl_VertexIndex + gl_InstanceIndex) * 1e-20, 1);
}
</pre>
Rendering is performed using single draw call for whole scene:
<pre>
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
</pre>

## GS max throughput when no output is produced

Geometry shader maximum throughput test uses empty geometry shader that produces no output:
<pre>
layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;
void main() {
}
</pre>
It is fed by empty vertex shader:
<pre>
void main() {
}
</pre>
Rendering is performed using single draw call for whole scene:
<pre>
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
</pre>

## GS max throughput when single constant triangle is produced

Geometry shader maximum throughput test uses the following geometry shader that produces single constant triangle:
<pre>
layout(triangles) in;
layout(triangle_strip,max_vertices=3) out;
void main() {
	gl_Position=vec4(0,0,0.5,1);
	EmitVertex();
	gl_Position=vec4(0,0,0.6,1);
	EmitVertex();
	gl_Position=vec4(0,1e-10,0.4,1);
	EmitVertex();
}
</pre>
It is fed by empty vertex shader:
<pre>
void main() {
}
</pre>
Rendering is performed using single draw call for whole scene:
<pre>
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
</pre>

## GS max throughput when two constant triangles are produced

Geometry shader maximum throughput test uses the following geometry shader that produces two constant triangles:
<pre>
layout(triangles) in;
layout(triangle_strip,max_vertices=6) out;
void main() {
	gl_Position=vec4(0,0,0.5,1);
	EmitVertex();
	gl_Position=vec4(0,0,0.6,1);
	EmitVertex();
	gl_Position=vec4(0,1e-10,0.4,1);
	EmitVertex();
	EndPrimitive();
	gl_Position=vec4(0,0,0.7,1);
	EmitVertex();
	gl_Position=vec4(0,0,0.8,1);
	EmitVertex();
	gl_Position=vec4(0,1e-10,0.9,1);
	EmitVertex();
}
</pre>
It is fed by empty vertex shader:
<pre>
void main() {
}
</pre>
Rendering is performed using single draw call for whole scene:
<pre>
vkCmdDraw(
	commandBuffer,
	3*numberOfTriangles,  // vertexCount
	1,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);
</pre>

## Instancing throughput of vkCmdDraw()

Instancing throughput uses single triangle instanced many times in single vkCmdDraw() call:
<pre>
vkCmdDraw(
	commandBuffer,
	3,  // vertexCount
	numberOfTriangles,  // instanceCount
	0,  // firstVertex
	0   // firstInstance
);  // vertexCount, instanceCount, firstVertex, firstInstance
</pre>
As the result, number of rendered triangles is equal to the number of instances processed.

The vertex shader in outputs constant coordinates, thus producing degenerated triangles:
<pre>
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
</pre>

## Instancing throughput of vkCmdDrawIndirect()

Instancing throughput uses single triangle instanced many times in single VkDrawIndirectCommand struct
with the following content:
<pre>
indirectBufferPtr->vertexCount = 3;
indirectBufferPtr->instanceCount = numberOfTriangles;
indirectBufferPtr->firstVertex = 0;
indirectBufferPtr->firstInstance = 0;
</pre>
The VkDrawIndirectCommand structure is processed by vkCmdDrawIndirect() call:
<pre>
vkCmdDrawIndirect(
	indirectBuffer,  // buffer
	0,  // offset
	1,  // drawCount
	sizeof(vk::DrawIndirectCommand)  // stride
);
</pre>

The vertex shader in use outputs constant coordinates, thus producing degenerated triangles:
<pre>
void main() {
	gl_Position = vec4(0, 0, 0.5, 1);
}
</pre>
