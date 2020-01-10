//This program implements a vector addition using OpenGL

#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h>

#include <CL/cl.h>

#define MEGABYTE 1048576
#define DEBUG

//OpenCL kernel to perform an element wise
//add of two arrays

const char* programSource =
"__kernel                                                \n"
"void vecadd(__global float *outputC,                    \n"
"            int widthA,                                 \n"
"            int heightA,                                \n"
"            int widthB,                                 \n"
"            int heightB,                                \n"
"            __global float *inputA,                     \n"
"            __global float *inputB)                     \n"
"{                                                       \n"
"                                                        \n"
"    //Get global position in Y direction                \n"
"    int row = get_global_id(1);                         \n"
"    //Get global position in X direction                \n"
"    int col = get_global_id(0);                         \n"
"                                                        \n"
"    float sum = 0.0f;                                   \n"
"                                                        \n"
"    //Calculate the result of one element iin Matrix C  \n"
"    for(int i=0; i < widthA; i++) {                     \n"
"      sum += inputA[row*widthA+i]*inputB[i*widthB+col]; \n"
"    }                                                   \n"
"                                                        \n"
"    outputC[row*widthB+col] = sum;                      \n"
"}                                                       \n"
;


#define WIDTH_A  4
#define HEIGHT_A 4
#define WIDTH_B  4
#define HEIGHT_B 4

int main() {
    //This code executes on the OpenGL host

    //Host data

    float *A = NULL;
    float *B = NULL;
    float *C = NULL;

    //Elements in each array
    const uint elements = WIDTH_A * WIDTH_B;

    //Compute the size of the data
    size_t datasize = sizeof(float)* elements;

    //Allocate space for input/output data
    A = (float*) malloc(datasize);
    B = (float*) malloc(datasize);
    C = (float*) malloc(datasize);
    //Initialize the input data
    for(int i=0; i < elements; i++)
    {
         A[i] = (float)  i;
         printf("[%g]", A[i]);
         if(i%WIDTH_A == WIDTH_A-1){
             printf("\n");
         }
    }

    printf("\n");

    for(int i=0; i < elements; i++)
    {
         B[i] = (float)  i+1;
         printf("[%g]", B[i]);
         if(i%WIDTH_A == WIDTH_A-1){
             printf("\n");
         }
    }

    printf("\n");

    //Use this to check the output of each API call
    cl_int status;

    //------------------------------------------------------
    //STEP 1: Discover and initialize the platforms
    //------------------------------------------------------

    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;

    //Use clGetPlatformIDs() to retrieve the number of
    //platforms
    status = clGetPlatformIDs(0, NULL, &numPlatforms);

    //Allocate enough space for each platform
    platforms = 
       (cl_platform_id*) malloc(
           numPlatforms * sizeof(cl_platform_id));

    //Fill in platforms witth clGetPlatformsIDs()
    status = clGetPlatformIDs(numPlatforms, platforms,
               NULL);
    

    //------------------------------------------------------
    //STEP 2: Discover and initialize the devices
    //------------------------------------------------------

    cl_uint numDevices = 0;
    cl_device_id * devices = NULL;
    //Use clGetDeviceIDs() to retrieve the number of
    //devices present
    status = clGetDeviceIDs(
        platforms[0],
        CL_DEVICE_TYPE_ALL,
        0,
        NULL,
        &numDevices);
    //Allocate enough space for each device
    devices = 
        (cl_device_id*) malloc(
           numDevices * sizeof(cl_device_id));

    //Fill in devices with clGetDeviceIDs()      
    status = clGetDeviceIDs(
        platforms[0],
        CL_DEVICE_TYPE_ALL,
        numDevices,
        devices,
        NULL);

    //------------------------------------------------------
    // STEP 3: Create a context
    //------------------------------------------------------
    cl_context context = NULL;
  
    //Create a context using clCreateContext() and
    //associate it with the devices
    context = clCreateContext(
        NULL,
        numDevices,
        devices,
        NULL,
        NULL,
        &status);

    //------------------------------------------------------
    // STEP 4: Create a command queue
    //------------------------------------------------------

    cl_command_queue cmdQueue;
    //Create a command queue using clCreateCommandQueue(),
    //and associate it with the device you want to execute
    //on
    cmdQueue = clCreateCommandQueue(
    context,
    devices[0],
    0,
    &status);

    //------------------------------------------------------
    // STEP 5: Create device buffers
    //------------------------------------------------------

    cl_mem inputA; //Input array on the device
    cl_mem inputB; //Input array on the device
    cl_mem outputC; //Input array on the device
    
    //Use cl_createBuffer() to create a buffer object (d_A)
    //that will contain the data from the host array A
    inputA = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY,
        datasize,
        NULL,
        &status);

    //Use cl_createBuffer() to create a buffer object (d_B)
    //that will contain the data from the host array B
    inputB = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY,
        datasize,
        NULL,
        &status);

    //Use cl_createBuffer() to create a buffer object (d_C)
    //that will contain the data from the host array C
    outputC = clCreateBuffer(
        context,
        CL_MEM_WRITE_ONLY,
        datasize,
        NULL,
        &status);

    //------------------------------------------------------
    // STEP 6: Write host data to device buffers
    //------------------------------------------------------

    //Use clEnqueueWriteBuffer() to write input arrayA to
    //the device buffer inputA
    status = clEnqueueWriteBuffer(
        cmdQueue,
        inputA,
        CL_FALSE,
        0,
        datasize,
        A,
        0,
        NULL,
        NULL);

    //Use clEnqueueWriteBuffer() to write input arrayB to
    //the device buffer inputB
    status = clEnqueueWriteBuffer(
        cmdQueue,
        inputB,
        CL_FALSE,
        0,
        datasize,
        B,
        0,
        NULL,
        NULL);

    //------------------------------------------------------
    // STEP 7: Create and compile the program
    //------------------------------------------------------

    //Create a program using clCreateProgramWithSource()
    cl_program program = clCreateProgramWithSource(
        context,
        1,
        (const char**) &programSource,
        NULL,
        &status);

     //Build (compile) the program for the devices with
     //clBuildProgram()
     status = clBuildProgram(
         program,
         numDevices,
         devices,
         NULL,
         NULL,
         NULL);
         
    //------------------------------------------------------
    // STEP 8: Create the kernel
    //------------------------------------------------------

    cl_kernel kernel = NULL;

    //Use clCreateKernel() to create a kernel from the
    //vector addition function (named "vecadd")
    kernel = clCreateKernel(program, "vecadd", &status);

    //------------------------------------------------------
    // STEP 9: Set the kernel arguments
    //------------------------------------------------------
    int widthA = WIDTH_A;
    int heightA = HEIGHT_A;
    int widthB = WIDTH_A;
    int heightB = HEIGHT_B;

    //Associate the input and output buffers with the
    //kernel
    //using clSetKernelArg()
    status = clSetKernelArg(
        kernel,
        0,
        sizeof(cl_mem),
        &outputC);
    status = clSetKernelArg(
        kernel,
        1,
        sizeof(int),
        &widthA);
    status = clSetKernelArg(
        kernel,
        2,
        sizeof(int),
        &heightA);
    status = clSetKernelArg(
        kernel,
        3,
        sizeof(int),
        &widthB);
    status = clSetKernelArg(
        kernel,
        4,
        sizeof(int),
        &heightB);
    status |= clSetKernelArg(
        kernel,
        5,
        sizeof(cl_mem),
        &inputA);
    status |= clSetKernelArg(
        kernel,
        6,
        sizeof(cl_mem),
        &inputB);

    //------------------------------------------------------
    // STEP 10: Configure the work-item structure
    //------------------------------------------------------
    
    //Define an index space (global work size) of work
    //items for execution. A workgroup size (local worksize)
    //is not required, but can be used.
    size_t globalWorkSize[1];
    globalWorkSize[0] = elements;

    //------------------------------------------------------
    // STEP 11: Enqueue the kernel for execution
    //------------------------------------------------------

    //Execute the kernel by using clEnqueueNDRangeKernel().
    //'globalWorkSize' is the 1D dimension 
    // of the work items
    status = clEnqueueNDRangeKernel(
        cmdQueue,
        kernel,
        1,
        NULL,
        globalWorkSize,
        NULL,
        0,
        NULL,
        NULL);

    
    //------------------------------------------------------
    // STEP 12: Read the output buffer back to the host
    //------------------------------------------------------

    //Use clEnqueueReadBuffer() to read the OpenCL output
    // buffer (outputC) to the host output array (C)
    clEnqueueReadBuffer(
         cmdQueue,
         outputC,
         CL_TRUE,
         0,
         datasize,
         C,
         0,
         NULL,
         NULL);

     //Verify the output
     bool result = true;
     int  error_elem_pos = 0;
     int error_elem_val = -1;

     for(int i = 0; i < elements; i++) {
         printf("[%g]", C[i]);
         if(i%WIDTH_A == WIDTH_A-1){
             printf("\n");
         }
     }

    printf("\n");

    //------------------------------------------------------
    // STEP 13: Release OpenCL resources
    //------------------------------------------------------

    //Free OpenCL resources
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(inputA);
    clReleaseMemObject(inputB);
    clReleaseMemObject(outputC);
    clReleaseContext(context);

    //Free host resources
    free(A);
    free(B);
    free(C);
    free(platforms);
    free(devices); 
}
