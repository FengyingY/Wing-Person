#pragma once

#include "Shapes.hpp"

namespace Collisions {
	
	//one shape colliding with one other of the same shape:
	bool circle_circle_collision(Shapes::Circle c1, Shapes::Circle c2);
	bool rectangle_rectangle_collision(Shapes::Rectangle r1, Shapes::Rectangle r2);
	bool triangle_triangle_collision(Shapes::Triangle t1, Shapes::Triangle t2);
	glm::vec2 circle_circle_collision(Shapes::Circle c1, Shapes::Circle c2, int axis);
	glm::vec2 rectangle_rectangle_collision(Shapes::Rectangle r1, Shapes::Rectangle r2, int axis);
	//glm::vec2 triangle_triangle_collision(Shapes::Triangle t1, Shapes::Triangle t2, int axis);
	

	//one circle colliding with one other of a different shape:
	bool circle_rectangle_collision(Shapes::Circle c, Shapes::Rectangle r);
	bool circle_triangle_collision(Shapes::Circle c, Shapes::Triangle t);
	glm::vec2 circle_rectangle_collision(Shapes::Circle c, Shapes::Rectangle r, int axis);
	//glm::vec2 circle_triangle_collision(Shapes::Circle c, Shapes::Triangle t, int axis);
	
	//one rectangle colliding with one other of a different shape:
	bool circle_rectangle_collision(Shapes::Rectangle r, Shapes::Circle c);
	bool rectangle_triangle_collision(Shapes::Rectangle r, Shapes::Triangle t);
	glm::vec2 circle_rectangle_collision(Shapes::Rectangle r, Shapes::Circle c, int axis);
	//glm::vec2 rectangle_triangle_collision(Shapes::Rectangle r, Shapes::Triangle t, int axis);

	//one triangle colliding with one other of a different shape:
	bool circle_triangle_collision(Shapes::Triangle t, Shapes::Circle c);
	bool rectangle_triangle_collision(Shapes::Triangle t, Shapes::Rectangle r);
	//glm::vec2 circle_triangle_collision(Shapes::Triangle t, Shapes::Circle c, int axis);
	//glm::vec2 rectangle_triangle_collision(Shapes::Triangle t, Shapes::Rectangle r, int axis);

	//one player (assumed to be a rectangle) colliding with vectors of other shapes:
	std::vector< Shapes::Circle > player_circles_collision(Shapes::Rectangle player, std::vector < Shapes::Circle > circles);
	std::vector< Shapes::Rectangle > player_rectangles_collision(Shapes::Rectangle player, std::vector < Shapes::Rectangle > rectangles);
	std::vector< Shapes::Triangle > player_triangles_collision(Shapes::Rectangle player, std::vector < Shapes::Triangle > triangles);

	//one player (assumed to be a rectangle) with a new position they want to move to colliding vectors of other shapes:
	bool player_circles_collision(Shapes::Rectangle player, glm::vec2 new_pos, std::vector < Shapes::Circle > circles);
	bool player_rectangles_collision(Shapes::Rectangle player, glm::vec2 new_pos, std::vector < Shapes::Rectangle > rectangles);
	bool player_triangles_collision(Shapes::Rectangle player, glm::vec2 new_pos, std::vector < Shapes::Triangle > triangles);

	//allows for a second vector to compare against for collisions:
	std::vector< Shapes::Rectangle > player_rectangles_collision(Shapes::Rectangle player, std::vector < Shapes::Rectangle > rectangles1, std::vector < Shapes::Rectangle > rectangles2);
	bool player_rectangles_collision(Shapes::Rectangle player, glm::vec2 new_pos, std::vector < Shapes::Rectangle > rectangles1, std::vector < Shapes::Rectangle > rectangles2);

	//check if lines intersect (useful for collision detection between 2D shapes):
	glm::vec2 line_intersect(glm::vec2 line1_p1, glm::vec2 line1_p2, glm::vec2 line2_p1, glm::vec2 line2_p2);

	//check if point is in a line segment (useful for collision detection between 2D shapes):
	bool point_in_line_segment(glm::vec2 point, glm::vec2 line_p1, glm::vec2 line_p2);
}