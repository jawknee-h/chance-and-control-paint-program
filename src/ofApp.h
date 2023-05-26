#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		
		bool paint_is_on;

		ofFbo canvas_fbo;
		ofColor background_colour;
		ofImage grng;

		// For tracking cursor movement.
		glm::vec2 previous_mouse_pos;
		glm::vec2 gesture_direction;
		float gesture_magnitude;
		float prev_mouse_amt;
		float current_mouse_amt;

		// For screen mirroring/slicing.
		int slice_offset;
		int num_of_slices;

		// Brush functions.
		void eraser_brush(float x, float y);
		void simple_brush(float x, float y);
		void dynamic_example_brush(float x, float y, float speed, glm::vec2 direction);
		void feather_brush(float x, float y, float speed, glm::vec2 direction);
		void line_brush(float x, float y, float speed);
		void sand_worm_brush(float x, float y);
		void caterpillar_brush(float x, float y);
		
		void clearCanvas();

		enum brushes {
			simple,
			eraser,
			dynamic_example,
			feather,
			line,
			sand_worm,
			caterpillar,
		} current_brush;
};
