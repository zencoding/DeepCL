// Copyright Hugh Perkins 2014 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License, 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.

#include "Propagate1.h"
#include "stringhelper.h"
#include "StatefulTimer.h"

using namespace std;

#undef VIRTUAL
#undef STATIC
#define VIRTUAL
#define STATIC

VIRTUAL Propagate1::~Propagate1() {
    delete kernel;
}
VIRTUAL void Propagate1::propagate( int batchSize, CLWrapper *dataWrapper, CLWrapper *weightsWrapper, CLWrapper *biasWeightsWrapper,
    CLWrapper *resultsWrapper ) {
    kernel->in(batchSize)
        ->in( dim.inputPlanes )->in( dim.numFilters )
        ->in( dim.inputImageSize )->in( dim.filterSize )
       ->in( dim.padZeros ? 1 : 0 );
    kernel->input( dataWrapper );
    kernel->input( weightsWrapper);
    if( dim.biased ) kernel->input( biasWeightsWrapper );
    kernel->output( resultsWrapper );

    int globalSize = batchSize * dim.outputCubeSize;
    int workgroupsize = std::min( globalSize, cl->getMaxWorkgroupSize() );
    globalSize = ( ( globalSize + workgroupsize - 1 ) / workgroupsize ) * workgroupsize;
//    cout << "propagate1 globalsize " << globalSize << " workgroupsize " << workgroupsize << endl;

    kernel->run_1d( globalSize, workgroupsize );
    cl->finish();
    StatefulTimer::timeCheck("Propagate1::propagate after call propagate");
}
Propagate1::Propagate1( OpenCLHelper *cl, LayerDimensions dim, ActivationFunction const*fn ) :
        Propagate( cl, dim, fn )
            {

    std::string options = "-D " + fn->getDefineName();
    if( dim.biased ) {
         options += " -D BIASED";
    }
    // [[[cog
    // import stringify
    // stringify.write_kernel2( "kernel", "cl/propagate1.cl", "convolve_imagecubes_float2", 'options' )
    // ]]]
    // generated using cog, from cl/propagate1.cl:
    const char * kernelSource =  
    "// Copyright Hugh Perkins 2014, 2015 hughperkins at gmail\n" 
    "//\n" 
    "// This Source Code Form is subject to the terms of the Mozilla Public License,\n" 
    "// v. 2.0. If a copy of the MPL was not distributed with this file, You can\n" 
    "// obtain one at http://mozilla.org/MPL/2.0/.\n" 
    "\n" 
    "// expected defines:\n" 
    "// one of: [ TANH | RELU | LINEAR ]\n" 
    "// BIASED (or not)\n" 
    "\n" 
    "#ifdef TANH\n" 
    "    #define ACTIVATION_FUNCTION(output) (tanh(output))\n" 
    "#elif defined SCALEDTANH\n" 
    "    #define ACTIVATION_FUNCTION(output) ( 1.7159f * tanh( 0.66667f * output))\n" 
    "#elif SIGMOID\n" 
    "    #define ACTIVATION_FUNCTION(output) (1.0f / (1 + exp(-output)))\n" 
    "#elif defined RELU\n" 
    "    #define ACTIVATION_FUNCTION(output) (output> 0 ? output : 0)\n" 
    "#elif defined LINEAR\n" 
    "    #define ACTIVATION_FUNCTION(output) (output)\n" 
    "#endif\n" 
    "\n" 
    "// notes on non-odd filtersizes:\n" 
    "// for odd, imagesize and filtersize 3, padZeros = 0:\n" 
    "// output is a single square\n" 
    "// m and n should vary between -1,0,1\n" 
    "// for even, imagesize and filtersize 2, padzeros = 0\n" 
    "// output is a single square, which we can position at topleft or bottomrigth\n" 
    "// lets position it in bottomright\n" 
    "// then m and n should vary as -1,0\n" 
    "//\n" 
    "// for even, imagesize and filtersize 2, padzeros = 1\n" 
    "// output is 2 by 2\n" 
    "// well... if it is even:\n" 
    "// - if we are not padding zeros, then we simply move our filter around the image somehow\n" 
    "// - if we are padding zeros, then we conceptually pad the bottom and right edge of the image with zeros by 1\n" 
    "// filtersize remains the same\n" 
    "//      m will vary as -1,0,1\n" 
    "//       outputrow is fixed by globalid\n" 
    "//       inputrow should be unchanged...\n" 
    "// padzeros = 0:\n" 
    "//  x x .  . . .\n" 
    "//  x x .  . x x\n" 
    "//  . . .  . x x\n" 
    "// when filtersize even:\n" 
    "//    new imagesize = oldimagesize - filtersize + 1\n" 
    "// when filtersize odd:\n" 
    "//    x x x .\n" 
    "//    x x x .\n" 
    "//    x x x .\n" 
    "//    . . . .\n" 
    "//    new imagesize = oldimagesize - filtersize + 1\n" 
    "// padzeros = 1:\n" 
    "// x x\n" 
    "// x x . .   x x .    . . .     . . .\n" 
    "//   . . .   x x .    . x x     . . .\n" 
    "//   . . .   . . .    . x x     . . x x\n" 
    "// outrow=0 outrow=1  outrow=2      x x\n" 
    "// outcol=0 outcol=1  outcol=2    outrow=3\n" 
    "//                                outcol=3\n" 
    "// when filtersize is even, and padzeros, imagesize grows by 1 each time...\n" 
    "//    imagesize = oldimagesize + 1\n" 
    "// when filtersize is odd\n" 
    "//  x x x\n" 
    "//  x x x .   x x x    . . .\n" 
    "//  x x x .   x x x    . x x x\n" 
    "//    . . .   x x x    . x x x\n" 
    "//                       x x x\n" 
    "\n" 
    "// images are organized like [imageId][plane][row][col]\n" 
    "// filters are organized like [filterid][inplane][filterrow][filtercol]\n" 
    "// results are organized like [imageid][filterid][row][col]\n" 
    "// global id is organized like results, ie: [imageid][outplane][outrow][outcol]\n" 
    "// - no local memory used currently\n" 
    "// - each thread:\n" 
    "//     - loads a whole upstream cube\n" 
    "//     - loads a whole filter cube\n" 
    "//     - writes one output...\n" 
    "#ifdef ACTIVATION_FUNCTION // protect against not defined\n" 
    "void kernel convolve_imagecubes_float2( const int numExamples,\n" 
    "      const int numInputPlanes, const int numFilters,\n" 
    "      const int inputImageSize, const int filterSize, const int padZeros,\n" 
    "      global const float *images, global const float *filters,\n" 
    "#ifdef BIASED\n" 
    "global const float*biases,\n" 
    "#endif\n" 
    "    global float *results ) {\n" 
    "    int globalId = get_global_id(0);\n" 
    "\n" 
    "    const int evenPadding = filterSize % 2 == 0 ? 1 : 0;\n" 
    "\n" 
    "    int inputImageSizeSquared = inputImageSize * inputImageSize;\n" 
    "    int outputImageSize = padZeros ? inputImageSize + evenPadding : inputImageSize - filterSize + 1;\n" 
    "    int outputImageSizeSquared = outputImageSize * outputImageSize;\n" 
    "    int filterSizeSquared = filterSize * filterSize;\n" 
    "\n" 
    "    int outputImage2Id = globalId / outputImageSizeSquared;\n" 
    "    int exampleId = outputImage2Id / numFilters;\n" 
    "    int filterId = outputImage2Id % numFilters;\n" 
    "\n" 
    "    int inputCubeOffset = exampleId * numInputPlanes * inputImageSizeSquared;\n" 
    "    int filterCubeOffset = filterId * numInputPlanes * filterSizeSquared;\n" 
    "\n" 
    "    // intraimage coords\n" 
    "    int localid = globalId % outputImageSizeSquared;\n" 
    "    int outputRow = localid / outputImageSize;\n" 
    "    int outputCol = localid % outputImageSize;\n" 
    "\n" 
    "    int halfFilterSize = filterSize >> 1;\n" 
    "    float sum = 0;\n" 
    "    //  imagesize = oldimagesize\n" 
    "    int minm = padZeros ? max( -halfFilterSize, -outputRow ) : -halfFilterSize;\n" 
    "    int maxm = padZeros ? min( halfFilterSize - evenPadding, outputImageSize - 1 - outputRow  - evenPadding) : halfFilterSize - evenPadding;\n" 
    "    int minn = padZeros ? max( -halfFilterSize, -outputCol ) : - halfFilterSize;\n" 
    "    int maxn = padZeros ? min( halfFilterSize - evenPadding, outputImageSize - 1 - outputCol - evenPadding) : halfFilterSize - evenPadding;\n" 
    "    int inputPlane = 0;\n" 
    "//    float probe = 0;\n" 
    "    while( inputPlane < numInputPlanes ) {\n" 
    "        int inputImageOffset = inputCubeOffset + inputPlane * inputImageSizeSquared;\n" 
    "        int filterImageOffset = filterCubeOffset + inputPlane * filterSizeSquared;\n" 
    "        int m = minm;\n" 
    "        while( m <= maxm ) {\n" 
    "            int inputRow = outputRow + m + ( padZeros ? 0 : halfFilterSize );\n" 
    "            int inputimagerowoffset = inputImageOffset + inputRow * inputImageSize;\n" 
    "            int filterrowoffset = filterImageOffset + (m+halfFilterSize) * filterSize + halfFilterSize;\n" 
    "            int n = minn;\n" 
    "            while( n <= maxn ) {\n" 
    "                int inputCol = outputCol + n + ( padZeros ? 0 : halfFilterSize );\n" 
    "                if( exampleId < numExamples ) {\n" 
    "                    sum += images[ inputimagerowoffset + inputCol] * filters[ filterrowoffset + n ];\n" 
    "                }\n" 
    "                n++;\n" 
    "            }\n" 
    "            m++;\n" 
    "        }\n" 
    "        inputPlane++;\n" 
    "    }\n" 
    "\n" 
    "    if( exampleId < numExamples ) {\n" 
    "    #ifdef BIASED\n" 
    "        sum += biases[filterId];\n" 
    "    #endif\n" 
    "        results[globalId] = ACTIVATION_FUNCTION(sum);\n" 
    "    }\n" 
    "}\n" 
    "#endif\n" 
    "\n" 
    "";
    kernel = cl->buildKernelFromString( kernelSource, "convolve_imagecubes_float2", options, "cl/propagate1.cl" );
    // [[[end]]]
    //kernel = cl->buildKernel( "propagate1.cl", "convolve_imagecubes_float2", options );
    //kernel = cl->buildKernelFromString( kernelSource, "convolve_imagecubes_float2", options, kernelFilename );
}

