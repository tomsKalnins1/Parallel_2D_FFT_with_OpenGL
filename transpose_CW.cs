#version 460 core

layout(local_size_x = X_INVOCATIONS, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D image_O; //input_b for vertical fft in this shader
layout(rgba32f, binding = 1) uniform image2D image_T;

shared vec4 input_b[NUM_SAMPLES] ;
shared vec4 real_imag_buffer[NUM_SAMPLES] ;

uint num_samples = NUM_SAMPLES;

void transpose() {

    ivec2 texCoors = ivec2(gl_GlobalInvocationID.xy);

    for (uint i = 0; i < num_samples / gl_WorkGroupSize.x; i++) {

        uint index = texCoors.x * (num_samples / gl_WorkGroupSize.x) + i;
        ivec2 sample_img = ivec2(texCoors.y, texCoors.x * (num_samples / gl_WorkGroupSize.x) + i);

        input_b[index] = imageLoad(image_O, sample_img);
     //   synchronize()

    }
}





//-------------------------------------------------------------------------SYNCHRONIZE DATA ACCESS
void synchronize()
{

    barrier();
    memoryBarrierShared();
}


void main()
{

    transpose();

    ivec2 texCoors = ivec2(gl_GlobalInvocationID.xy);
    for (uint i = 0; i < num_samples / gl_WorkGroupSize.x; i++)
    {

        uint index = texCoors.x * (num_samples / gl_WorkGroupSize.x) + i;

        imageStore(image_T, ivec2(texCoors.x * (num_samples / gl_WorkGroupSize.x) + i, texCoors.y), input_b[index]);

    }



}
