/*
 * Zoom.h
 *
 *  Created on: 14 oct. 2017
 *      Author: Vincent
 */


#ifndef ZOOM_H_
#define ZOOM_H_

#include <stdint.h>

#define BASE_ZOOM_LEVEL    5
#define BASE_ZOOM_METERS   60.

class Zoom {
public:
	Zoom();

	void increaseZoom();
	void decreaseZoom();
	void resetZoom();

	void setSpan(uint16_t h_space, uint16_t v_space);
	void computeZoom(float lat, float distance, float& h_zoom, float& v_zoom);

	uint8_t getZoomLevel() const {
		return m_zoom_level;
	}

private:
	uint16_t m_h_size;
	uint16_t m_v_size;

	uint8_t m_zoom_level;
};


#endif /* ZOOM_H_ */

