/*
 * Filter.h
 *
 *  Created on: 12 juin 2017
 *      Author: Vincent
 */

#ifndef DRIVERS_FILTER_H_
#define DRIVERS_FILTER_H_


#include "Arduino.h"

typedef struct
{
	float input;
	float output;
	float *pState;
	float *pCoefficients;
} filter_executionState;



class Filter {
public:
	Filter(const float *coefs);
	void reset();
	void input(float input);
	float output();

private:
	void filterBiquad(filter_executionState * pExecState);

	float m_filter_coefficients[10];
	float m_state[8];
	float m_output;

};


#endif /* DRIVERS_FILTER_H_ */
