// Copyright Hugh Perkins 2014,2015 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License, 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.

#include <stdexcept>

#include "NeuralNet.h"
#include "FullyConnectedLayer.h"
#include "ConvolutionalLayer.h"
#include "InputLayer.h"
#include "SoftMaxLayer.h"
#include "SquareLossLayer.h"
#include "CrossEntropyLoss.h"
#include "PoolingLayer.h"
#include "NormalizationLayer.h"
#include "RandomPatches.h"
#include "RandomTranslations.h"

#include "LayerMaker.h"

using namespace std;

Layer *SquareLossMaker::createLayer( Layer *previousLayer ) {
    return new SquareLossLayer( previousLayer, this );
}
Layer *CrossEntropyLossMaker::createLayer( Layer *previousLayer ) {
    return new CrossEntropyLoss( previousLayer, this );
}
Layer *SoftMaxMaker::createLayer( Layer *previousLayer ) {
    return new SoftMaxLayer( previousLayer, this );
}


//Layer *LayerMaker::insert() {
//    Layer *layer = net->addLayer( this );
//    return layer;
//}
//Layer *FullyConnectedMaker::insert() {
//    if( _numPlanes == 0 ) {
//        throw runtime_error("Must provide ->planes(planes)");
//    }
//    if( _imageSize == 0 ) {
//        throw runtime_error("Must provide ->imageSize(imageSize)");
//    }
//    Layer *layer = net->addLayer( this );
//    return layer;
//}
//template< typename T > Layer *InputLayerMaker<T>::insert() {
//    if( _numPlanes == 0 ) {
//        throw runtime_error("Must provide ->numPlanes(planes)");
//    }
//    if( _imageSize == 0 ) {
//        throw runtime_error("Must provide ->imageSize(imageSize)");
//    }
//    Layer *layer = net->addLayer( this );
//    return layer;
//}
//Layer *ConvolutionalMaker::insert() {
//    if( _numFilters == 0 ) {
//        throw runtime_error("Must provide ->numFilters(numFilters)");
//    }
//    if( _filterSize == 0 ) {
//        throw runtime_error("Must provide ->filterSize(filterSize)");
//    }
//    Layer *layer = net->addLayer( this );
//    return layer;
//}

//template< typename T>
//InputLayerMaker<T> InputLayerMaker<T>::clone() const {
//    InputLayerMaker<T> maker = *this;
////    maker->_numPlanes = _numPlanes;
////    maker->_imageSize = _imageSize;
//    return maker;
//}
//LayerMaker *NormalizationLayerMaker::clone( Layer *previousLayer ) const {
//    NormalizationLayerMaker *maker = new NormalizationLayerMaker( 0, previousLayer );
//    maker->_translate = _translate;
//    maker->_scale = _scale;
//    return maker;
//}
//LayerMaker *RandomPatchesMaker::clone( Layer *previousLayer ) const {
//    RandomPatchesMaker *maker = new RandomPatchesMaker( 0, previousLayer );
//    maker->_patchSize = _patchSize;
//    return maker;
//}
//LayerMaker *RandomTranslationsMaker::clone( Layer *previousLayer ) const {
//    RandomTranslationsMaker *maker = new RandomTranslationsMaker( 0, previousLayer );
//    maker->_translateSize = _translateSize;
//    return maker;
//}
//LayerMaker *PoolingMaker::clone( Layer *previousLayer ) const {
//    PoolingMaker *maker = new PoolingMaker( 0, previousLayer );
//    maker->_poolingSize = _poolingSize;
//    maker->_padZeros = _padZeros;
//    return maker;
//}
//LayerMaker *FullyConnectedMaker::clone( Layer *previousLayer ) const {
//    FullyConnectedMaker *maker = new FullyConnectedMaker( 0, previousLayer );
//    maker->_numPlanes = _numPlanes;
//    maker->_imageSize = _imageSize;
//    maker->_biased = _biased;
//    maker->_activationFunction = _activationFunction;
//    return maker;
//}
//LayerMaker *SquareLossMaker::clone( Layer *previousLayer ) const {
//    SquareLossMaker *maker = new SquareLossMaker( 0, previousLayer );
//    return maker;
//}
//LayerMaker *CrossEntropyLossMaker::clone( Layer *previousLayer ) const {
//    CrossEntropyLossMaker *maker = new CrossEntropyLossMaker( 0, previousLayer );
//    return maker;
//}
//LayerMaker *SoftMaxMaker::clone( Layer *previousLayer ) const {
//    SoftMaxMaker *maker = new SoftMaxMaker( 0, previousLayer );
//    maker->_perPlane = _perPlane;
//    return maker;
//}
//LayerMaker *ConvolutionalMaker::clone( Layer *previousLayer ) const {
//    ConvolutionalMaker *maker = new ConvolutionalMaker( 0, previousLayer );
//    maker->_numFilters = _numFilters;
//    maker->_filterSize = _filterSize;
//    maker->_padZeros = _padZeros;
//    maker->_biased = _biased;
//    maker->_activationFunction = _activationFunction;
//    return maker;
//}

//Layer *FullyConnectedMaker::instance() const {
//    if( _numPlanes == 0 ) {
//        throw runtime_error("Must provide ->planes(planes)");
//    }
//    if( _imageSize == 0 ) {
//        throw runtime_error("Must provide ->imageSize(imageSize)");
//    }
//    Layer *layer = new FullyConnectedLayer( net->cl, previousLayer, this );
//    return layer;
//}
//Layer *SquareLossMaker::instance() const {
//    SquareLossLayer *layer = new SquareLossLayer( previousLayer, this );
//    return layer;
//}
//Layer *CrossEntropyLossMaker::instance() const {
//    CrossEntropyLoss *layer = new CrossEntropyLoss( previousLayer, this );
//    return layer;
//}
//Layer *SoftMaxMaker::instance() const {
//    Layer *layer = new SoftMaxLayer( previousLayer, this );
//    return layer;
//}
//Layer *PoolingMaker::instance() const {
//    Layer *layer = new PoolingLayer( previousLayer, this );
//    return layer;
//}
//template< typename T > Layer *InputLayerMaker<T>::instance() const {
//    if( _numPlanes == 0 ) {
//        throw runtime_error("Must provide ->numPlanes(planes)");
//    }
//    if( _imageSize == 0 ) {
//        throw runtime_error("Must provide ->imageSize(imageSize)");
//    }
//    Layer *layer = new InputLayer<T>( 0, this );
//    return layer;
//}
//Layer *NormalizationLayerMaker::instance() const {
//    Layer *layer = new NormalizationLayer( previousLayer, this );
//    return layer;
//}
//Layer *RandomPatchesMaker::instance() const {
//    Layer *layer = new RandomPatches( previousLayer, this );
//    return layer;
//}
//Layer *RandomTranslationsMaker::instance() const {
//    Layer *layer = new RandomTranslations( previousLayer, this );
//    return layer;
//}

//int ConvolutionalMaker::getOutputImageSize() const {
//    if( previousLayer == 0 ) {
//        throw std::runtime_error("convolutional network must be attached to a parent layer");
//    }
//    int evenPadding = _filterSize % 2 == 0 ? 1 : 0;
//    int imageSize = _padZeros ? previousLayer->getOutputImageSize() + evenPadding : previousLayer->getOutputImageSize() - _filterSize + 1;
//    return imageSize;
//}

//int LossLayerMaker::getOutputImageSize() const {
//    return previousLayer->getOutputImageSize();
//}
//int LossLayerMaker::getOutputPlanes() const {
//    return previousLayer->getOutputPlanes();
//}
//int LossLayerMaker::getBiased() const {
//    return previousLayer->getBiased();
//}
//int PoolingMaker::getOutputImageSize() const {
//    return previousLayer->getOutputImageSize() / _poolingSize;
//}
//int PoolingMaker::getOutputPlanes() const {
//    return previousLayer->getOutputPlanes();
//}
//int PoolingMaker::getBiased() const {
//    return false;
//}
//int NormalizationLayerMaker::getOutputImageSize() const {
//    return previousLayer->getOutputImageSize();
//}
//int NormalizationLayerMaker::getOutputPlanes() const {
//    return previousLayer->getOutputPlanes();
//}
//int NormalizationLayerMaker::getBiased() const {
//    return false;
//}
//int RandomPatchesMaker::getOutputImageSize() const {
//    return _patchSize;
//}
//int RandomPatchesMaker::getOutputPlanes() const {
//    return previousLayer->getOutputPlanes();
//}
//int RandomPatchesMaker::getBiased() const {
//    return false;
//}

//int RandomTranslationsMaker::getOutputImageSize() const {
//    return previousLayer->getOutputImageSize();
//}
//int RandomTranslationsMaker::getOutputPlanes() const {
//    return previousLayer->getOutputPlanes();
//}
//int RandomTranslationsMaker::getBiased() const {
//    return false;
//}


