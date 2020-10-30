#include "Collisions.hpp"

//NOTE: For now it is assumed that touching is a collision. Code can be changed so it would only ne a collision if the shapes are inside one another

//check for collision between two circles:
bool Collisions::circle_circle_collision(Shapes::Circle c1, Shapes::Circle c2) {
	//get the distance between the centers of the circles:
	float center_dist = std::sqrt(std::pow(c1.center.x - c2.center.x, 2) + std::pow(c1.center.y - c2.center.y, 2));
	
	//check for collision:
	if (center_dist <= c1.radius + c2.radius)
		return true;

	//no collision found:
	return false;
}

//check for collision between two rectangles:
bool Collisions::rectangle_rectangle_collision(Shapes::Rectangle r1, Shapes::Rectangle r2) {
	//check for collision:
	for (glm::vec2 corner : r1.corners) {
		if (corner.x <= r2.corners[0].x && corner.x >= r2.corners[1].x &&
			corner.y <= r2.corners[0].y && corner.y >= r2.corners[2].y)
			return true;
	}

	//could still be collisions if r2 is contained by r1, so check if the dimensions of the rectangles allow this:
	if (r1.width <= r2.width && r1.height <= r2.height)
		return false;

	//r2 could be contained by r1, so check for collision:
	for (glm::vec2 corner : r2.corners) {
		if (corner.x <= r1.corners[0].x && corner.x >= r1.corners[1].x &&
			corner.y <= r1.corners[0].y && corner.y >= r1.corners[2].y)
			return true;
	}
	
	//no collision found:
	return false;
}

//check for collision between two triangles:
bool Collisions::triangle_triangle_collision(Shapes::Triangle t1, Shapes::Triangle t2) {
	//TODO: make sure one of the triangles does not contain the other
	
	//if the intersect point is on one of the rectangle's and one of the triangle's line segments, then there is an intersect:
	for (int i = 0; i < 3; i++) {
		int next_point_i = (i + 1) % 3;
		for (int j = 0; j < 3; j++) {
			int next_point_j = (j + 1) % 3;
			glm::vec2 intersect = line_intersect(t1.points[i], t1.points[next_point_i], t2.points[j], t2.points[next_point_j]);
			if (point_in_line_segment(intersect, t1.points[i], t1.points[next_point_i]) &&
				point_in_line_segment(intersect, t2.points[j], t2.points[next_point_j]))
				return true;
		}
	}

	//no collision found:
	return false;
}

//check for collision between one circle and one rectangle:
bool Collisions::circle_rectangle_collision(Shapes::Circle c, Shapes::Rectangle r) {
	//get the distance between the centers of the shapes:
	float center_dist = std::sqrt(std::pow(c.center.x - r.center.x, 2) + std::pow(c.center.y - r.center.y, 2));

	//if the circle radius and rectangle corner distance are too large, there is no chance for a collision:
	if (center_dist > c.radius + r.corner_dist)
		return false;

	//if the center of the circle is contained by the rectangle, then there is a collision:
	if (c.center.x <= r.corners[0].x && c.center.x >= r.corners[1].x &&
		c.center.y <= r.corners[0].y && c.center.y >= r.corners[2].y)
		return true;

	//get the points of intersection for the edges of the rectangle and the line made by the shape centers:
	std::vector< glm::vec2 > intersections;
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[0], r.corners[1]));
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[1], r.corners[2]));
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[2], r.corners[3]));
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[3], r.corners[0]));

	//check for collision:
	for (glm::vec2 intersect : intersections) {
		float intersect_dist = std::sqrt(std::pow(intersect.x - r.center.x, 2) + std::pow(intersect.y - r.center.y, 2));
		if (center_dist <= c.radius + intersect_dist)
			return true;
	}

	//no collision found:
	return false;
}

//check for collision betwwen one circle and one triangle:
bool Collisions::circle_triangle_collision(Shapes::Circle c, Shapes::Triangle t) {
	//check for collision:
	for (glm::vec2 point : t.points) {
		float dist = std::sqrt(std::pow(c.center.x - point.x, 2) + std::pow(c.center.y - point.y, 2));
		if (dist <= c.radius)
			return true;
	}

	//no collision found:
	return false;
}

//check for collision between one rectangle and one triangle:
bool Collisions::rectangle_triangle_collision(Shapes::Rectangle r, Shapes::Triangle t) {
	//if at least one of the points of the triangle is inside the rectangle, then there is a collision:
	for (glm::vec2 point : t.points) {
		if (point.x <= r.corners[0].x && point.x >= r.corners[1].x &&
			point.y <= r.corners[0].y && point.y >= r.corners[2].y)
			return true;
	}
	
	//if the intersect point is on one of the rectangle's and one of the triangle's line segments, then there is an intersect:
	for (int i = 0; i < 4; i++) {
		int next_corner = (i + 1) % 4;
		for (int j = 0; j < 3; j++) {
			int next_point = (j + 1) % 3;
			glm::vec2 intersect = line_intersect(r.corners[i], r.corners[next_corner], t.points[j], t.points[next_point]);
			if (point_in_line_segment(intersect, r.corners[i], r.corners[next_corner]) &&
				point_in_line_segment(intersect, t.points[j], t.points[next_point]))
				return true;
		}
	}

	//no collision found:
	return false;
}

//check if one player (assumed to be a rectangle) collides with any circle in the list:
bool Collisions::player_circles_collision(Shapes::Rectangle player, std::vector < Shapes::Circle > circles) {
	//check for collisions:
	for (Shapes::Circle circle : circles) {
		if (circle_rectangle_collision(circle, player))
			return true;
	}
	
	//no collisions found:
	return false;
}

//check if one player (assumed to be a rectangle) collides with any rectangle in the list:
bool Collisions::player_rectangles_collision(Shapes::Rectangle player, std::vector < Shapes::Rectangle > rectangles) {
	//check for collisions:
	for (Shapes::Rectangle rectangle : rectangles) {
		if (rectangle_rectangle_collision(player, rectangle))
			return true;
	}

	//no collisions found:
	return false;
}

//check if one player (assumed to be a rectangle) collides with any triangle in the list:
bool Collisions::player_triangles_collision(Shapes::Rectangle player, std::vector < Shapes::Triangle > triangles) {
	//check for collisions:
	for (Shapes::Triangle triangle : triangles) {
		if (rectangle_triangle_collision(player, triangle))
			return true;
	}

	//no collisions found:
	return false;
}

//check if one player (assumed to be a rectangle) with a new position collides with any circle in the list:
bool Collisions::player_circles_collision(Shapes::Rectangle player, glm::vec2 new_pos, std::vector < Shapes::Circle > circles) {
	//create new rectangle for new position:
	Shapes::Rectangle new_player_pos = Shapes::Rectangle(new_pos, player.width, player.height);
	
	//check for collisions:
	for (Shapes::Circle circle : circles) {
		if (circle_rectangle_collision(circle, new_player_pos))
			return true;
	}

	//no collisions found:
	return false;
}

//check if one player (assumed to be a rectangle) with a new position collides with any rectangle in the list:
bool Collisions::player_rectangles_collision(Shapes::Rectangle player, glm::vec2 new_pos, std::vector < Shapes::Rectangle > rectangles) {
	//create new rectangle for new position:
	Shapes::Rectangle new_player_pos = Shapes::Rectangle(new_pos, player.width, player.height);
	
	//check for collisions:
	for (Shapes::Rectangle rectangle : rectangles) {
		if (rectangle_rectangle_collision(new_player_pos, rectangle))
			return true;
	}

	//no collisions found:
	return false;
}

//check if one player (assumed to be a rectangle) with a new position collides with any triangle in the list:
bool Collisions::player_triangles_collision(Shapes::Rectangle player, glm::vec2 new_pos, std::vector < Shapes::Triangle > triangles) {
	//create new rectangle for new position:
	Shapes::Rectangle new_player_pos = Shapes::Rectangle(new_pos, player.width, player.height);
	
	//check for collisions:
	for (Shapes::Triangle triangle : triangles) {
		if (rectangle_triangle_collision(new_player_pos, triangle))
			return true;
	}

	//no collisions found:
	return false;
}

//code for this function came from: https://www.geeksforgeeks.org/program-for-point-of-intersection-of-two-lines/
glm::vec2 Collisions::line_intersect(glm::vec2 line1_p1, glm::vec2 line1_p2, glm::vec2 line2_p1, glm::vec2 line2_p2) {
	//line1 represented as a1x + b1y = c1 
	float a1 = line1_p2.y - line1_p1.y;
	float b1 = line1_p1.x - line1_p2.x;
	float c1 = (a1 * line1_p1.x) + (b1 * line1_p1.y);

	//line2 represented as a2x + b2y = c2 
	float a2 = line2_p2.y - line2_p1.y;
	float b2 = line2_p1.x - line2_p2.x;
	float c2 = (a2 * line2_p1.x) + (b2 * line2_p1.y);

	//used to help determine if there is an intersection and if so, where:
	float determinant = a1 * b2 - a2 * b1;

	//return the point of intersection:
	if (determinant == 0) {
		//the lines are parallel (represented by a point with half the maximum float value for x and y)
		return glm::vec2(std::numeric_limits<float>::max() / 2.0f, std::numeric_limits<float>::max() / 2.0f);
	} else {
		float x = (b2 * c1 - b1 * c2) / determinant;
		float y = (a1 * c2 - a2 * c1) / determinant;
		return glm::vec2(x, y);
	}
}

//code for this function came from: https://stackoverflow.com/questions/1585459/whats-the-most-efficient-way-to-detect-triangle-triangle-intersections
bool Collisions::point_in_line_segment(glm::vec2 point, glm::vec2 line_p1, glm::vec2 line_p2) {
	//get the distances between the three points:
	float point_p1_dist = std::sqrt(std::pow(point.x - line_p1.x, 2) + std::pow(point.y - line_p1.y, 2));
	float point_p2_dist = std::sqrt(std::pow(point.x - line_p2.x, 2) + std::pow(point.y - line_p2.y, 2));
	float line_dist = std::sqrt(std::pow(line_p1.x - line_p2.x, 2) + std::pow(line_p1.y - line_p2.y, 2));

	//check if point is in the line segment:
	if (point_p1_dist + point_p2_dist == line_dist)
		return true;

	//point is not in the line segment:
	return false;
}