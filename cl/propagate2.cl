// Copyright Hugh Perkins 2014, 2015 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License, 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.

// expected defines:
// one of: [ TANH | RELU | LINEAR ]
// BIASED (or not)

#ifdef TANH
    #define ACTIVATION_FUNCTION(output) (tanh(output))
#elif defined SCALEDTANH
    #define ACTIVATION_FUNCTION(output) ( 1.7159f * tanh( 0.66667f * output))
#elif SIGMOID
    #define ACTIVATION_FUNCTION(output) (1.0f / (1 + exp(-output)))
#elif defined RELU
    #define ACTIVATION_FUNCTION(output) (output> 0 ? output : 0)
#elif defined LINEAR
    #define ACTIVATION_FUNCTION(output) (output)
#endif

#ifdef gOutputBoardSize // for previous tests that dont define it
#ifdef ACTIVATION_FUNCTION // protect against not defined
// workgroup id organized like: [outplane]
// local id organized like: [outrow][outcol]
// each thread iterates over: [imageid][upstreamplane][filterrow][filtercol]
// number workgroups = 32
// one filter plane takes up 5 * 5 * 4 = 100 bytes
// one filter cube (corresponding to one outplane) = 5*5 * 32 * 4 = 3.2KB (ok)
// all filter cubes = 3.2KB * 32 = 102KB (too big)
// results are organized like [imageid][filterid][row][col]
// assumes filter is small, so filtersize * filterSize * inputPlanes * 4 < about 3KB
//                            eg 5 * 5 * 32 * 4 = 3.2KB => ok :-)
//                           but 28 * 28 * 32 * 4 = 100KB => less good :-P
void kernel propagate_2_by_outplane( const int batchSize,
      global const float *images, global const float *filters, 
        #ifdef BIASED
            global const float*biases, 
        #endif
    global float *results,
    local float *_upstreamBoard, local float *_filterCube ) {
    const int globalId = get_global_id(0);

//    const int evenPadding = gFilterSize % 2 == 0 ? 1 : 0;

    const int workgroupId = get_group_id(0);
    const int workgroupSize = get_local_size(0);
    const int outPlane = workgroupId;

    const int localId = get_local_id(0);
    const int outputRow = localId / gOutputBoardSize;
    const int outputCol = localId % gOutputBoardSize;

    const int minu = gPadZeros ? max( -gHalfFilterSize, -outputRow ) : -gHalfFilterSize;
    const int maxu = gPadZeros ? min( gHalfFilterSize - gEven, gOutputBoardSize - 1 - outputRow  - gEven) : gHalfFilterSize - gEven;
    const int minv = gPadZeros ? max( -gHalfFilterSize, -outputCol ) : - gHalfFilterSize;
    const int maxv = gPadZeros ? min( gHalfFilterSize - gEven, gOutputBoardSize - 1 - outputCol - gEven) : gHalfFilterSize - gEven;

    const int numUpstreamsPerThread = ( gInputBoardSizeSquared + workgroupSize - 1 ) / workgroupSize;

    const int filterCubeLength = gInputPlanes * gFilterSizeSquared;
    const int filterCubeGlobalOffset = outPlane * filterCubeLength;
    const int numPixelsPerThread = ( filterCubeLength + workgroupSize - 1 ) / workgroupSize;
    for( int i = 0; i < numPixelsPerThread; i++ ) {
        int thisOffset = localId + i * workgroupSize;
        if( thisOffset < filterCubeLength ) {
            _filterCube[thisOffset] = filters[filterCubeGlobalOffset + thisOffset];
        }
    }
    // dont need a barrier, since we'll just run behind the barrier from the upstream board download

    for( int n = 0; n < batchSize; n++ ) {
        float sum = 0;
        for( int upstreamPlane = 0; upstreamPlane < gInputPlanes; upstreamPlane++ ) {
            int thisUpstreamBoardOffset = ( n * gInputPlanes + upstreamPlane ) * gInputBoardSizeSquared;
            barrier(CLK_LOCAL_MEM_FENCE);
            for( int i = 0; i < numUpstreamsPerThread; i++ ) {
                int thisOffset = workgroupSize * i + localId;
                if( thisOffset < gInputBoardSizeSquared ) {
                    _upstreamBoard[ thisOffset ] = images[ thisUpstreamBoardOffset + thisOffset ];
                }
            }
            barrier(CLK_LOCAL_MEM_FENCE);
            int filterBoardOffset = upstreamPlane * gFilterSizeSquared;
            if( localId < gOutputBoardSizeSquared ) {
                for( int u = minu; u <= maxu; u++ ) {
                    int inputRow = outputRow + u + ( gPadZeros ? 0 : gHalfFilterSize );
                    int inputboardrowoffset = inputRow * gInputBoardSize;
                    int filterrowoffset = filterBoardOffset + (u+gHalfFilterSize) * gFilterSize + gHalfFilterSize;
                    for( int v = minv; v <= maxv; v++ ) {
                        int inputCol = outputCol + v + ( gPadZeros ? 0 : gHalfFilterSize );
                        sum += _upstreamBoard[ inputboardrowoffset + inputCol] * _filterCube[ filterrowoffset + v ];
                    }
                }
            }
        }
        #ifdef BIASED
            sum += biases[outPlane];
        #endif
        // results are organized like [imageid][filterid][row][col]
        int resultIndex = ( n * gNumFilters + outPlane ) * gOutputBoardSizeSquared + localId;
        if( localId < gOutputBoardSizeSquared ) {
            results[resultIndex ] = ACTIVATION_FUNCTION(sum);
        }
    }
}
#endif
#endif

