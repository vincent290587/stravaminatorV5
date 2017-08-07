/*
 * Filter.cpp
 *
 *  Created on: 12 juin 2017
 *      Author: Vincent
 */

#include "Filter.h"
#include "string.h"

Filter::Filter(const float *coefs) {

	memcpy(m_filter_coefficients, coefs, 10*sizeof(float));

	this->reset();
}


void Filter::reset() {

	memset(m_state, 0, 8*sizeof(float));

	m_output = 0.;
}


float Filter::output() {
	return m_output;
}


void Filter::input(float input)
{
	filter_executionState executionState;          // The executionState structure holds call data, minimizing stack reads and writes
	executionState.input = input;                 // Pointers to the input and output buffers that each call to filterBiquad() will use
	executionState.output = m_output;               // - pInput and pOutput can be equal, allowing reuse of the same memory.
	executionState.pState = m_state;                   // Pointer to the biquad's internal state and coefficients.
	executionState.pCoefficients = m_filter_coefficients;    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad

	// The 1st call to cur_filter_filterBiquad() reads from the caller supplied input buffer and writes to the output buffer.
	// The remaining calls to filterBiquad() recycle the same output buffer, so that multiple intermediate buffers are not required.
	// Run biquad #0
	this->filterBiquad( &executionState );
	// The remaining biquads will now re-use the same output buffer.
	executionState.input = executionState.output;
	// Run biquad #1
	this->filterBiquad( &executionState );

	// store result
	m_output = executionState.output;

	// At this point, the caller-supplied output buffer will contain the filtered samples and the input buffer will contain the unmodified input samples.
	return;

}

void Filter::filterBiquad(filter_executionState * pExecState)
{
	// Read state variables
	float w0, x0;
	float w1 = pExecState->pState[0];
	float w2 = pExecState->pState[1];

	// Read coefficients into work registers
	float b0 = *(pExecState->pCoefficients++);
	float b1 = *(pExecState->pCoefficients++);
	float b2 = *(pExecState->pCoefficients++);
	float a1 = *(pExecState->pCoefficients++);
	float a2 = *(pExecState->pCoefficients++);

	float accumulator;

	// Read input sample
	x0 = pExecState->input;

	// Run feedback part of filter
	accumulator  = w2 * a2;
	accumulator += w1 * a1;
	accumulator += x0 ;

	w0 = accumulator ;

	// Run feedforward part of filter
	accumulator  = w0 * b0;
	accumulator += w1 * b1;
	accumulator += w2 * b2;

	w2 = w1;    // Shuffle history buffer
	w1 = w0;

	// Write output
	pExecState->output = accumulator;

	// Write state variables
	*(pExecState->pState++) = w1;
	*(pExecState->pState++) = w2;

}


