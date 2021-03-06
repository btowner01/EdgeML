// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#ifndef __QUANTIZED_FASTGRNN_H__
#define __QUANTIZED_FASTGRNN_H__

#define ERR_PRECOMP_NOT_INIT -1
#define ERR_TEMPLRW_NOT_INIT -2
#define ERR_TEMPLRU_NOT_INIT -3
#define ERR_NORMFEATURES_NOT_INIT -4

#include "quantized_utils.h"

/**
 * @brief Model paramters for low-rank FastGRNN
 * @var       mean         pointer to mean of input vector for normalization, size inputDims
 * @var       stdDev       pointer to standard dev of input for normalization, size inputDims*steps
 * @var       W1           pointer to first low-rank component of W
 * @var       W2           pointer to second low-rank component of W
 * @var       wRank        rank of W matrix
 * @var       U1           pointer to first low-rank component of U
 * @var       U2           pointer to second low-rank component of U
 * @var       uRank        rank of U matrix
 * @var       Bg           pointer to bias for sigmoid
 * @var       Bh           pointer to bias for tanh
 * @var       sigmoid_zeta first weight parameter for update from input from next step
 * @var       sigmoid_nu   second weight parameter for update from input from next step
 */
typedef struct Q_FastGRNN_LR_Params {
  INT_T* mean;
  INT_T* stdDev;
  INT_T* W1;
  INT_T* W2;
  ITER_T wRank;
  INT_T* U1;
  INT_T* U2;
  ITER_T uRank;
  INT_T* Bg;
  INT_T* Bh;
  INT_T sigmoid_zeta;
  INT_T sigmoid_nu;
} Q_FastGRNN_LR_Params;

/**
 * @brief Model scales for different inputs. The naming convention follows
 * two basic rules:
 * 1) If the matrix for which the scale is associated is used only once, name
 * the scale after the matrix.
 * 2) If the matrix for which the scale is associated is used only once, name
 * the scale after the matrix, the operation it is being used under and the
 * matrix it is being operated with.
 */
typedef struct Q_FastGRNN_LR_Scales {
  SCALE_T input;
  SCALE_T mean;
  SCALE_T meanSub;
  SCALE_T stdDev;
  SCALE_T normFeaturesHDStdDev;
  SCALE_T W1;
  SCALE_T normFeaturesMVW1;
  SCALE_T H1W1;
  SCALE_T H2W1;
  SCALE_T W2;
  SCALE_T tempLRW;
  SCALE_T H1W2;
  SCALE_T H2W2;
  SCALE_T U1;
  SCALE_T hiddenStateMVU1;
  SCALE_T H1U1;
  SCALE_T H2U1;
  SCALE_T U2;
  SCALE_T tempLRU;
  SCALE_T H1U2;
  SCALE_T H2U2;
  SCALE_T mV2AddMV4;
  SCALE_T mV4AddMV2;
  SCALE_T mV2AddMV4Out;
  SCALE_T pC1AddBg;
  SCALE_T Bg;
  SCALE_T pC1AddBgOut;
  SCALE_T sigmoidScaleIn;
  SCALE_T sigmoidScaleOut;
  SCALE_T pC1AddBh;
  SCALE_T Bh;
  SCALE_T pC1AddBhOut;
  SCALE_T tanhScaleIn;
  SCALE_T tanhScaleOut;
  SCALE_T gateHDHiddenState;
  SCALE_T hiddenStateHDGate;
  SCALE_T qOneScale;
  SCALE_T qOneSubGate;
  SCALE_T qOneSubGateOut;
  SCALE_T sigmoidZeta;
  SCALE_T sigmoidZetaMulQOneSubGate;
  SCALE_T sigmoidNu;
  SCALE_T sigmoidNuAddQOneSubGate;
  SCALE_T sigmoidNuAddQOneSubGateOut;
  SCALE_T sigmoidNuAddQOneSubGateHDUpdate;
  SCALE_T updateHDSigmoidNuAddQOneSubGate;
  SCALE_T pC3AddPC1;
  SCALE_T pC1AddPC3;
  SCALE_T hiddenStateOut;
  INT_T sigmoidLimit;
  INT_T div;
  INT_T add;
  INT_T qOne;
} Q_FastGRNN_LR_Scales;

/**
* @brief Buffers required for computation of low-rank FastGRNN
* @var   preComp1     pointer to buffer space, must be initalized to atleast hiddenDims size
* @var   preComp2     pointer to buffer space, must be initalized to atleast hiddenDims size
* @var   preComp3     pointer to buffer space, must be initalized to atleast hiddenDims size
* @var   tempLRW      pointer to buffer space, must be initalized to atleast wRank size
* @var   tempLRU      pointer to buffer space, must be initalized to atleast uRank size
* @var   normFeatures pointer to buffer space, must be initalized to atleast inputDims size
*/
typedef struct Q_FastGRNN_LR_Buffers {
  INT_T* preComp1;
  INT_T* preComp2;
  INT_T* preComp3;
  INT_T* tempLRW;
  INT_T* tempLRU;
  INT_T* normFeatures;
} Q_FastGRNN_LR_Buffers;

/**
 * @brief Multi-step updates of a FastGRNN cell with low rank W, U(W=W1*W2; U=U1*U2)
 * @param[in,out]   hiddenState  pointer to initial hidden state and output hidden state
 * @param[in]       hiddenDims   dimension of hidden state of the FastGRNN cell
 * @param[in]       input        pointer to concatenated input vectors for all steps, size inputDims*steps
 * @param[in]       inputDims    dimension of input vector for each step
 * @param[in]       steps        number of steps of FastGRNN cell
 * @param[in]       params       pointer to model parameter
 * @param[in]       buffers      pointer to buffer spaces
 * @param[in]       scales       pointer to model scales
 * @param[in]       backward     direction of the pass, 0 for forward, 1 for backward
 * @param[in]       normalize    apply mean-var normalization, 0 for no, 1 for yes
 * @return     The function returns <code>0</code> on success
 *             <code>ERR_PRECOMP_NOT_INIT</code> if preComp1 not allocated
 *             <code>ERR_PRECOMP_NOT_INIT</code> if preComp2 not allocated
 *             <code>ERR_PRECOMP_NOT_INIT</code> if preComp3 not allocated
 *             <code>ERR_TEMPLRW_NOT_INIT</code> if tempLRW not allocated
 *             <code>ERR_TEMPLRU_NOT_INIT</code> if tempLRU not allocated
 *             <code>ERR_NORMFEAT_NOT_INIT</code> if normFeatures not allocated
*/
int q_fastgrnn_lr(INT_T* const hiddenState, ITER_T hiddenDims,
                  const INT_T* const input, ITER_T inputDims, ITER_T steps,
                  const void* params, void* buffers, const void* scales,
                  int backward, int normalize);

/**
 * @brief Model paramters for low-rank FastGRNN
 * @var       mean         pointer to mean of input vector for normalization, size inputDims
 * @var       stdDev       pointer to standard dev of input for normalization, size inputDims*steps
 * @var       W            pointer to W matrix
 * @var       U            pointer U matrix
 * @var       Bg           pointer to bias for sigmoid
 * @var       Bh           pointer to bias for tanh
 * @var       sigmoid_zeta first weight parameter for update from input from next step
 * @var       sigmoid_nu   second weight parameter for update from input from next step
 */
typedef struct Q_FastGRNN_Params {
  INT_T* mean;
  INT_T* stdDev;
  INT_T* W;
  INT_T* U;
  INT_T* Bg;
  INT_T* Bh;
  INT_T sigmoid_zeta;
  INT_T sigmoid_nu;
} Q_FastGRNN_Params;

/**
 * @brief Model scales for different inputs. The naming convention follows
 * two basic rules:
 * 1) If the matrix for which the scale is associated is used only once, name
 * the scale after the matrix.
 * 2) If the matrix for which the scale is associated is used only once, name
 * the scale after the matrix, the operation it is being used under and the
 * matrix it is being operated with.
 */
typedef struct Q_FastGRNN_Scales {
  SCALE_T input;
  SCALE_T mean;
  SCALE_T meanSub;
  SCALE_T stdDev;
  SCALE_T normFeaturesHDStdDev;
  SCALE_T W;
  SCALE_T normFeaturesMVW;
  SCALE_T H1W;
  SCALE_T H2W;
  SCALE_T U;
  SCALE_T hiddenStateMVU;
  SCALE_T H1U;
  SCALE_T H2U;
  SCALE_T mV1AddMV2;
  SCALE_T mV2AddMV1;
  SCALE_T mV1AddMV2Out;
  SCALE_T pC1AddBg;
  SCALE_T Bg;
  SCALE_T pC1AddBgOut;
  SCALE_T sigmoidScaleIn;
  SCALE_T sigmoidScaleOut;
  SCALE_T pC1AddBh;
  SCALE_T Bh;
  SCALE_T pC1AddBhOut;
  SCALE_T tanhScaleIn;
  SCALE_T tanhScaleOut;
  SCALE_T gateHDHiddenState;
  SCALE_T hiddenStateHDGate;
  SCALE_T qOneScale;
  SCALE_T qOneSubGate;
  SCALE_T qOneSubGateOut;
  SCALE_T sigmoidZeta;
  SCALE_T sigmoidZetaMulQOneSubGate;
  SCALE_T sigmoidNu;
  SCALE_T sigmoidNuAddQOneSubGate;
  SCALE_T sigmoidNuAddQOneSubGateOut;
  SCALE_T sigmoidNuAddQOneSubGateHDUpdate;
  SCALE_T updateHDSigmoidNuAddQOneSubGate;
  SCALE_T pC3AddPC1;
  SCALE_T pC1AddPC3;
  SCALE_T hiddenStateOut;
  INT_T div;
  INT_T add;
  INT_T sigmoidLimit;
  INT_T qOne;
} Q_FastGRNN_Scales;

/**
* @brief Buffers required for computation of FastGRNN
* @var   preComp1     pointer to buffer space, must be initalized to atleast hiddenDims size
* @var   preComp2     pointer to buffer space, must be initalized to atleast hiddenDims size
* @var   preComp3     pointer to buffer space, must be initalized to atleast hiddenDims size
* @var   normFeatures pointer to buffer space, must be initalized to atleast inputDims size
*/
typedef struct Q_FastGRNN_Buffers {
  INT_T* preComp1;
  INT_T* preComp2;
  INT_T* preComp3;
  INT_T* normFeatures;
} Q_FastGRNN_Buffers;

/**
 * @brief Multi-step updates of a FastGRNN cell
 * @param[in,out]   hiddenState  pointer to initial hidden state and output hidden state
 * @param[in]       hiddenDims   dimension of hidden state of the FastGRNN cell
 * @param[in]       input        pointer to concatenated input vectors for all steps, size inputDims*steps
 * @param[in]       inputDims    dimension of input vector for each step
 * @param[in]       steps        number of steps of FastGRNN cell
 * @param[in]       params       pointer to model parameter
 * @param[in]       buffers      pointer to buffer spaces
 * @param[in]       scales       pointer to model scales
 * @param[in]       backward     direction of the pass, 0 for forward, 1 for backward
 * @param[in]       normalize    apply mean-var normalization, 0 for no, 1 for yes
 * @return     The function returns <code>0</code> on success
 *             <code>ERR_PRECOMP_NOT_INIT</code> if preComp1 not allocated
 *             <code>ERR_PRECOMP_NOT_INIT</code> if preComp2 not allocated
 *             <code>ERR_PRECOMP_NOT_INIT</code> if preComp3 not allocated
 *             <code>ERR_NORMFEAT_NOT_INIT</code> if normFeatures not allocated
 * @example          Please refer the file: c_reference/tests/fastgrnn/test_quantized_fastgrnn.c
 */
int q_fastgrnn(INT_T* const hiddenState, ITER_T hiddenDims,
               const INT_T* const input, ITER_T inputDims, ITER_T steps,
               const void* params, void* buffers, const void* scales,
               int backward, int normalize);

#endif
