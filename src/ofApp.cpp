/*
Johnny Henson
October 2022

Paint Program

Features:
	(1) 7 different brushes:
		- simple
		- eraser
		- dynamic
		- feather
		- line
		- sand worm
		- caterpillar

	(2) Canvas mirroring
		- Press '+' to split the canvas, press '-' to remove a split.

	(3) Canvas shifting
		- Press '[' to shift the canvas left, press ']' to shift the canvas right.
		- The effect is especially cool when the canvas has already been split/mirrored a few times.
*/

#include "ofApp.h"

using namespace glm;

//--------------------------------------------------------------
void ofApp::setup(){

	// Stored so the eraser knows which colour to use.
	background_colour = ofColor(41, 40, 39, 255);

	// Loading the grunge overlay texture.
	grng.load("grng.jpg");

	paint_is_on = false;
	current_brush = simple;

	// Initialising the slicing/tiling variables.
	slice_offset = 0;
	num_of_slices = 1;

	// Initialising the dynamic brush variables.
	previous_mouse_pos = vec2(0, 0);
	gesture_direction = vec2(0, 0);
	gesture_magnitude = 0;
	prev_mouse_amt = 0.8;	// smoothing amt.
	current_mouse_amt = 1 - prev_mouse_amt;

	// Allocating and clearing the FBO to begin with.
	canvas_fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA, 4);
	canvas_fbo.begin();
	ofClear(background_colour);
	canvas_fbo.end();
}

//--------------------------------------------------------------
void ofApp::update(){

	// Drawing to the FBO.
	canvas_fbo.begin();

		// Calculating the velocity of mouse movement this frame (gesture) by
		// subtracting the previous mouse position vector from current mouse position.
		vec2 mouse_pos = vec2(mouseX, mouseY);
		vec2 gesture = mouse_pos - previous_mouse_pos;
		gesture_magnitude = length(gesture);
		gesture_direction = normalize(gesture);

		// Updating previous_mouse_pos to hold our current mouse x and y.
		previous_mouse_pos.x = prev_mouse_amt * previous_mouse_pos.x + mouseX * current_mouse_amt;
		previous_mouse_pos.y = prev_mouse_amt * previous_mouse_pos.y + mouseY * current_mouse_amt;

		// Brush drawing code here.
		if (paint_is_on)
		{
			// Translating to account for the slice_offset.
			// Otherwise the brush would not actually draw where my cursor was
			// when the offset was anything but 0.
			ofTranslate(slice_offset, 0);
			
			// Drawing with the appropriate brush.
			switch (current_brush)
			{
				case eraser:
					eraser_brush(mouseX, mouseY);
					break;
				case simple:
					simple_brush(mouseX, mouseY);
					break;
				case dynamic_example:
					dynamic_example_brush(previous_mouse_pos.x, previous_mouse_pos.y, gesture_magnitude, gesture_direction);
					break;
				case feather:
					feather_brush(previous_mouse_pos.x, previous_mouse_pos.y, gesture_magnitude, gesture_direction);
					break;
				case line:
					line_brush(previous_mouse_pos.x, previous_mouse_pos.y, gesture_magnitude);
					break;
				case sand_worm:
					sand_worm_brush(previous_mouse_pos.x, previous_mouse_pos.y);
					break;
				case caterpillar:
					caterpillar_brush(previous_mouse_pos.x, previous_mouse_pos.y);
					break;
			}
		}
	canvas_fbo.end();
}

//--------------------------------------------------------------
void ofApp::draw(){

	// Drawing a subsection of the canvas_fbo, with the exact subsection dimensions
	// depending on the num_of_slices variable.
	// Repeatedly tiling that one subsection.
	// If num_of_slices == 1, the fbo will draw normally.
	for (int d = 0; d < num_of_slices; d++)
	{
		canvas_fbo.getTexture().drawSubsection(canvas_fbo.getWidth()/num_of_slices * d, 0, canvas_fbo.getWidth() / num_of_slices, canvas_fbo.getHeight(), slice_offset, 0);
	}

	// Drawing the grunge texture overlay.
	ofPushStyle();
		ofSetColor(255, 255, 255, 30);
		grng.draw(0, 0);
	ofPopStyle();
}

/// Draws an ellipse with the same fill colour as the background, to act as an eraser.
/// param x - a float representing the x coordinate that the brush should be drawn.
/// param y - a float representing the y coordinate that the brush should be drawn.
/// void - returns nothing.
void ofApp::eraser_brush(float x, float y)
{
	ofSetColor(background_colour);
	ofDrawEllipse(x, y, 30, 30);
}

/// Draws a simple black ellipse brush.
/// param x - a float representing the x coordinate that the brush should be drawn.
/// param y - a float representing the y coordinate that the brush should be drawn.
/// void - returns nothing.
void ofApp::simple_brush(float x, float y)
{
	ofSetColor(0);
	ofDrawEllipse(x, y, 30, 30);
}

/// Draws a straight line brush. The line will deviate from its normal position noisily,
/// with deviation strength being relative to the speed param.
/// There is another line underneath which has a line width which scales with speed,
/// creating a drop-shadow/trail effect when speed is high.
/// param x - a float representing the x coordinate that the brush should be drawn.
/// param y - a float representing the y coordinate that the brush should be drawn.
/// param speed - a float representing the current speed of the mouse.
/// void - returns nothing.
void ofApp::line_brush(float x, float y, float speed)
{
	ofNoFill();
	ofSetLineWidth(1);

	// Using noise to offset the brush x pos.
	float n = ofNoise(x, y);
	float noise_intensity = speed * 0.3;
	x += n * noise_intensity;

	// Rotating, then drawing a line.
	ofPushMatrix();
	ofTranslate(x, y);
	ofRotate(90);

	// Drawing a light coloured line with width which changes based on speed.
	ofSetLineWidth(speed * 0.1);
	ofSetColor(179, 175, 168, 40);
	ofDrawLine(0, 0, 30, 30);

	// Draw a thin line directly on top with a dark colour.
	ofSetLineWidth(1);
	ofSetColor(0);
	ofDrawLine(0, 0, 30, 30);

	ofPopMatrix();
}

/// Draws a brush which mirrors itself along x = ofGetWidth()/2
/// param x - a float representing the x coordinate that the brush should be drawn.
/// param y - a float representing the y coordinate that the brush should be drawn.
/// void - returns nothing.
void ofApp::caterpillar_brush(float x, float y)
{
	ofNoFill();

	// Getting the absolute distance of x from the center of the canvas.
	float mirror_line = ofGetWidth() / 2;
	float _x = abs(mirror_line - x);

	// Looping twice and drawing the brush at its position relative to the center of the canvas
	// At the end of the first loop, _x (the absolute distance of the mouse from the center of the canvas)
	// gets inverted, resulting in the brush being mirrored in the second loop.
	for (int m = 0; m < 2; m++)
	{
		ofPushMatrix();
			// Translating _x pixels from the center.
			ofTranslate(ofGetWidth() / 2 + _x, y);

			// Drawing the brush twice with low opacity to give it a smokey glow effect.
			for (int d = -1; d < 2; d += 2)
			{
				ofPushMatrix();
					// Inner ellipse.
					ofSetLineWidth(1);
					ofSetColor(0, 0, 0, 255);
					ofEllipse(0, 0, 20, 20);

					// Outer ellipse.
					ofSetColor(179, 175, 168, 40);
					ofSetLineWidth(0.1);
					ofEllipse(0, 0, 30, 30);
				ofPopMatrix();
			}
			// Flipping _x so that on the next loop
			// the brush will be drawn on the other side of the window.
			_x = -_x;
		ofPopMatrix();
	}
}

/// Draws a brush which mirrors itself along x = ofGetWidth()/2
/// param x - a float representing the x coordinate that the brush should be drawn.
/// param y - a float representing the y coordinate that the brush should be drawn.
/// void - returns nothing.
void ofApp::sand_worm_brush(float x, float y)
{
	ofNoFill();

	// Getting the absolute distance of x from the center of the canvas.
	float mirror_line = ofGetWidth() / 2;
	float _x = abs(mirror_line - x);

	// Looping twice and drawing the brush at its position relative to the center of the canvas
	// At the end of the first loop, _x (the absolute distance of the mouse from the center of the canvas)
	// gets inverted, resulting in the brush being mirrored in the second loop.
	for (int m = 0; m < 2; m ++)
	{
		ofPushMatrix();
			// Translating _x pixels from the center.
			ofTranslate(ofGetWidth() / 2 + _x, y);

			// Drawing the brush.
			// Inner ellipse.
			ofSetLineWidth(20);
			ofSetColor(0, 0, 0, 255);
			ofEllipse(0, 0, 20, 20);

			// Outer ellipse.
			ofSetColor(179, 175, 168, 40);
			ofSetLineWidth(0.1);
			ofEllipse(0, 0, 30, 30);

			// Flipping _x so that on the next loop
			// the brush will be drawn on the other side of the window.
			_x = -_x;
		ofPopMatrix();
	}
}

/// Draws a brush which mirrors itself along x = ofGetWidth()/2. The brush resembles a feather.
/// param x - a float representing the x coordinate that the brush should be drawn.
/// param y - a float representing the y coordinate that the brush should be drawn.
/// param speed - a float representing the distance between the mouse position and previous position.
/// param direction - the direction of acceleration of the mouse.
/// void - returns nothing.
void ofApp::feather_brush(float x, float y, float speed, vec2 direction)
{
	ofNoFill();

	// Calculating the change in direction of the mouse.
	vec2 drawing_dir = vec2(0, -1);

	// Rotation angle between "forward" of drawing and mouse moved direction.
	float angle = atan2(direction.x, direction.y) - atan2(drawing_dir.x, drawing_dir.y);

	// Getting the absolute distance of x from the center of the canvas.
	float _x = abs((ofGetWidth() / 2) - x);
		

	// Mapping the alpha of the brush to the speed parameter passed in.
	ofSetColor(179, 175, 168, ofMap(speed, 0, 100, 255, 10, true));
	ofSetLineWidth(1);

	// Looping twice to draw the brush mirrored on both sides of the canvas.
	// The first time the lines are drawn in a light colour
	// with a thin stroke width; the second time the lines are drawn on top
	// in a dark colour with a thin stroke width.
	for (int i = 0; i < 2; i++)
	{
		ofPushMatrix();
			ofTranslate(ofGetWidth() / 2 + _x, y);	// Translating _x pixels from the center.
			// Multiplying the rotation by -sign(_x) to make sure the rotation is mirrored accordingly.
			// if _x is a positive number, -sign(_x) will return -1.
			// if _x is a negative number, -sign(_x) will return 1.
			ofRotateRad(-angle * -sign(_x));
		
			// Mapping the angle and length of the line to the the speed parameter.
			// The effect is that the lines will point more narrowly as if they're
			// trying to be more aerodynamic, and will appear to stretch out to
			// emphasize speed.
			float line_angle = ofMap(speed, 0, 100, PI / 3, PI / 20, true);
			float line_length = ofMap(speed, 0, 100, 10, 50);

			// Looping twice. Drawing an angled line like half an arrowhead
			// and multiplying its rotation by d.
			// d is -1 in the first loop, and 1 in the second loop.
			// This effectively just draws that same half of an arrow head but mirrored.
			for (int d = -1; d < 2; d += 2)
			{
				ofPushMatrix();
				ofRotateRad(line_angle * d);
				ofDrawLine(0, 0, 0, line_length);
				ofPopMatrix();
			}
			// Flipping _x so that on the next loop
			// the brush will be drawn on the other side of the window.
			_x = -_x;
		ofPopMatrix();

		// Adjusting colours for the second loop,
		// this way the two mirrored halves are drawn in different colours.
		ofSetColor(0, 0, 0, ofMap(speed, 0, 100, 255, 10, true));
		ofSetLineWidth(1);
	}
}

/// Draws a triangle and line brush which faces the direction it's moving.
/// param x - a float representing the x coordinate that the brush should be drawn.
/// param y - a float representing the y coordinate that the brush should be drawn.
/// param speed - a float representing the distance between the mouse position and previous position.
/// param direction - the direction of acceleration of the mouse.
/// void - returns nothing.
void ofApp::dynamic_example_brush(float x, float y, float speed, vec2 direction)
{
	ofNoFill();
	ofSetLineWidth(1);

	// Map speed parameter to scale, clamp it in this range.
	float scale = ofMap(speed, 1, 50, 0.1, 3, true);

	// "forward" vector of my triangle drawing.
	vec2 drawing_dir = vec2(0, -1);
	
	// Find the angle between the forward vector (drawing_dir) and mouse moved direction.
	float angle = atan2(direction.x, direction.y) - atan2(drawing_dir.x, drawing_dir.y);	// in radians.

	// Scale the line based on speed.
	// (multiply unit vector by clamped value)
	vec2 limited = direction * ofClamp(speed, 1, 30);

	// Translate to mouse position, then draw.
	ofPushMatrix();
		ofTranslate(x, y);
		ofSetColor(129, 179, 37, 60);
		ofScale(scale);
		ofRotateRad(-angle); // -angle aligns the rotation direction with the direction of the mouse.
		ofDrawLine(0, 0, limited.x, limited.y);
		ofDrawTriangle(5, 10, -5, 10, 0, 0);
	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	cout << char(key) << endl;
	// Switching Brushes.
	switch (char(key))
	{
		case 'e':
			current_brush = eraser;
			break;
		case '1':
			current_brush = simple;
			break;
		case '2':
			current_brush = dynamic_example;
			break;
		case '3':
			current_brush = feather;
			break;
		case '4':
			current_brush = line;
			break;
		case '5':
			current_brush = sand_worm;
			break;
		case '6':
			current_brush = caterpillar;
			break;
		case '[':
			slice_offset -= 10;
			break;
		case ']':
			slice_offset += 10;
			break;
		case '-':
			num_of_slices -= 1;
			break;
		case '=':
			num_of_slices += 1;
		default:
			cout << "There is no brush bound to the " << "'" << char(key) << "'" << " key." << endl;
			break;
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	paint_is_on = true;
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	paint_is_on = false;
}