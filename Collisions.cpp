#include "Collisions.hpp"

#include <iostream>

//NOTE: For now it is assumed that touching without overlap is not a collision. Code can be changed so it would only ne a collision if the shapes are inside one another

//one shape colliding with one other of the same shape:

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
//   std::cout << "Rect1 [" << r1.center.x << ", " << r1.center.y << "] [" << r1.width << ", " << r1.height << "]" << std::endl;
//   std::cout << "Rect2 [" << r2.center.x << ", " << r2.center.y << "] [" << r2.width << ", " << r2.height << "]" << std::endl;
	//check for collision:
	for (glm::vec2 corner : r1.corners) {
		if (corner.x <= r2.corners[0].x && corner.x >= r2.corners[1].x &&
			corner.y <= r2.corners[0].y && corner.y >= r2.corners[2].y) {
    //   std::cout << "True" << std::endl << std::endl;
			return true;
    }
	}

	//could still be collisions if r2 is contained by r1, so check if the dimensions of the rectangles allow this:
	if (r1.width <= r2.width && r1.height <= r2.height) {
    //   std::cout << "False" << std::endl << std::endl;
		return false;
  }

	//r2 could be contained by r1, so check for collision:
	for (glm::vec2 corner : r2.corners) {
		if (corner.x <= r1.corners[0].x && corner.x >= r1.corners[1].x &&
			corner.y <= r1.corners[0].y && corner.y >= r1.corners[2].y) {
			return true;
    }
	}

	//no collision found:
	return false;
}

//check for collision between two triangles:
bool Collisions::triangle_triangle_collision(Shapes::Triangle t1, Shapes::Triangle t2) {
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

//check for collision between two circles:
glm::vec2 Collisions::circle_circle_collision(Shapes::Circle c1, Shapes::Circle c2, int axis) {
	//get the distance between the centers of the circles:
	float center_dist = std::sqrt(std::pow(c1.center.x - c2.center.x, 2) + std::pow(c1.center.y - c2.center.y, 2));

	//check for collision:
	if (center_dist < c1.radius + c2.radius) {

		//get the adjusted distance vector between the centers:
		glm::vec2 avg_diff = c1.center - c2.center;
		float total_diff = std::abs(avg_diff.x) + std::abs(avg_diff.y);
		if (total_diff != 0.0f) {
			avg_diff.x = avg_diff.x / total_diff;
			avg_diff.y = avg_diff.y / total_diff;
		}

		//get the desired distance between centers and adjust the avg_diff as needed:
		float desired_dist;
		if (axis == 0) {
			if (avg_diff.x < 0.0f)
				avg_diff.x = -1.0f;
			else
				avg_diff.x = 1.0f;
			avg_diff.y = 0.0f;
			
			float radius_dist = c1.radius + c2.radius;
			float y_dist = c1.center.y - c2.center.y;

			desired_dist = std::sqrt(std::pow(radius_dist, 2) - std::pow(y_dist, 2)) * avg_diff.x;
		}
		else if (axis == 1) {
			avg_diff.x = 0.0f;
			if (avg_diff.y < 0.0f)
				avg_diff.y = -1.0f;
			else
				avg_diff.y = 1.0f;

			float radius_dist = c1.radius + c2.radius;
			float x_dist = c1.center.x - c2.center.x;

			desired_dist = std::sqrt(std::pow(radius_dist, 2) - std::pow(x_dist, 2)) * avg_diff.y;
		}
		else {
			desired_dist = c1.radius + c2.radius;
		}
		

		//get the vector that points between c2's center and the desired position for c1's center:
		float vector_from_c2_x;
		float vector_from_c2_y;
		if (avg_diff.x == 0.0f) {
			vector_from_c2_x = 0.0f;
			vector_from_c2_y = desired_dist;
		}
		else if (avg_diff.y == 0.0f) {
			vector_from_c2_x = desired_dist;
			vector_from_c2_y = 0.0f;
		}
		else {
			float ratio_avg_diff = avg_diff.y / avg_diff.x;
			vector_from_c2_x = (desired_dist * ratio_avg_diff) / sqrt(1.0f + std::pow(ratio_avg_diff, 2));
			vector_from_c2_y = vector_from_c2_x / ratio_avg_diff;
		}
		glm::vec2 vector_from_c2 = glm::vec2(vector_from_c2_x, vector_from_c2_y);

		//I was asked to return how much the first object would have to move to be out of the second object, but I could just return desired_center
		glm::vec2 desired_center = c2.center + vector_from_c2;
		return desired_center - c1.center;
	}

	//no collision found:
	return glm::vec2(0.0f, 0.0f);
}

//check for collision between two rectangles:
glm::vec2 Collisions::rectangle_rectangle_collision(Shapes::Rectangle r1, Shapes::Rectangle r2, int axis) {
  	//check for collision:
	for (int i = 0; i < 4; i++) {
		if (r1.corners[i].x < r2.corners[0].x && r1.corners[i].x > r2.corners[1].x &&
			r1.corners[i].y < r2.corners[0].y && r1.corners[i].y > r2.corners[2].y) {

			//get the adjusted distance vector between the centers:
			glm::vec2 avg_diff = r1.center - r2.center;
			float total_diff = std::abs(avg_diff.x) + std::abs(avg_diff.y);
			if (total_diff != 0.0f) {
				avg_diff.x = avg_diff.x / total_diff;
				avg_diff.y = avg_diff.y / total_diff;
			}

			//get the desired distance between centers and adjust the avg_diff as needed:
			float desired_dist;
			if (axis == 0) {
				if (avg_diff.x < 0.0f)
					avg_diff.x = -1.0f;
				else
					avg_diff.x = 1.0f;
				avg_diff.y = 0.0f;
				desired_dist = (r1.width / 2.0f) + (r2.width / 2.0f);
			}
			else if (axis == 1) {
				avg_diff.x = 0.0f;
				if (avg_diff.y < 0.0f)
					avg_diff.y = -1.0f;
				else
					avg_diff.y = 1.0f;
				desired_dist = (r1.height / 2.0f) + (r2.height / 2.0f);
			}
			else {
				if (std::abs(avg_diff.x) > std::abs(avg_diff.y))
					desired_dist = (r1.width / 2.0f) + (r2.width / 2.0f);
				else if (std::abs(avg_diff.x) > std::abs(avg_diff.y))
					desired_dist = (r1.height / 2.0f) + (r2.height / 2.0f);
				else
					desired_dist = r1.corner_dist + r2.corner_dist;
			}

			//get the vector that points between r2's center and the desired position for r1's center:
			float vector_from_r2_x;
			float vector_from_r2_y;
			if (avg_diff.x == 0.0f) {
				vector_from_r2_x = 0.0f;
				vector_from_r2_y = desired_dist;
			}
			else if (avg_diff.y == 0.0f) {
				vector_from_r2_x = desired_dist;
				vector_from_r2_y = 0.0f;
			}
			else {
				float ratio_avg_diff = avg_diff.y / avg_diff.x;
				vector_from_r2_x = (desired_dist * ratio_avg_diff) / sqrt(1.0f + std::pow(ratio_avg_diff, 2));
				vector_from_r2_y = vector_from_r2_x / ratio_avg_diff;
			}
			glm::vec2 vector_from_r2 = glm::vec2(vector_from_r2_x, vector_from_r2_y);

			//I was asked to return how much the first object would have to move to be out of the second object, but I could just return desired_center
			glm::vec2 desired_center = r2.center + vector_from_r2;
			(void) desired_center;
  
		}
	}

	//no collision found:
	return glm::vec2(0.0f, 0.0f);
}

//one circle colliding with one other of a different shape:

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
		if (dist < c.radius)
			return true;
	}

	//no collision found:
	return false;
}

//check for collision between one circle and one rectangle:
glm::vec2 Collisions::circle_rectangle_collision(Shapes::Circle c, Shapes::Rectangle r, int axis) {
	//get the distance between the centers of the shapes:
	float center_dist = std::sqrt(std::pow(c.center.x - r.center.x, 2) + std::pow(c.center.y - r.center.y, 2));

	//if the circle radius and rectangle corner distance are too large, there is no chance for a collision:
	if (center_dist > c.radius + r.corner_dist)
		return glm::vec2(0.0f, 0.0f);

	/*if the center of the circle is contained by the rectangle, then there is a collision:
	if (c.center.x < r.corners[0].x && c.center.x > r.corners[1].x &&
		c.center.y < r.corners[0].y && c.center.y > r.corners[2].y) {

		//get the adjusted distance vector between the centers:
		glm::vec2 avg_diff = c.center - r.center;
		float total_diff = std::abs(avg_diff.x) + std::abs(avg_diff.y);
		if (total_diff != 0.0f) {
			avg_diff.x = avg_diff.x / total_diff;
			avg_diff.y = avg_diff.y / total_diff;
		}

		//get the desired distance between centers:
		float desired_dist;
		if (avg_diff.x > avg_diff.y)
			desired_dist = c.radius + (r.width / 2.0f);
		else if (avg_diff.x < avg_diff.y)
			desired_dist = c.radius + (r.height / 2.0f);
		else
			desired_dist = c.radius + r.corner_dist;

		//get the vector that points between r's center and the desired position for c's center:
		float vector_from_r_x;
		float vector_from_r_y;
		if (avg_diff.x == 0.0f) {
			vector_from_r_x = 0.0f;
			vector_from_r_y = desired_dist;
		}
		else if (avg_diff.y == 0.0f) {
			vector_from_r_x = desired_dist;
			vector_from_r_y = 0.0f;
		}
		else {
			float ratio_avg_diff = avg_diff.y / avg_diff.x;
			vector_from_r_x = (desired_dist * ratio_avg_diff) / sqrt(1.0f + std::pow(ratio_avg_diff, 2));
			vector_from_r_y = vector_from_r_x / ratio_avg_diff;
		}
		glm::vec2 vector_from_r = glm::vec2(vector_from_r_x, vector_from_r_y);

		//I was asked to return how much the first object would have to move to be out of the second object, but I could just return desired_center
		glm::vec2 desired_center = r.center + vector_from_r;
		return desired_center - c.center;
	}*/

	//get the points of intersection for the edges of the rectangle and the line made by the shape centers:
	std::vector< glm::vec2 > intersections;
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[0], r.corners[1]));
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[1], r.corners[2]));
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[2], r.corners[3]));
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[3], r.corners[0]));

	//check for collision:
	for (glm::vec2 intersect : intersections) {
		float intersect_dist = std::sqrt(std::pow(intersect.x - r.center.x, 2) + std::pow(intersect.y - r.center.y, 2));
		if (center_dist < c.radius + intersect_dist) {

			//get the adjusted distance vector between the centers:
			glm::vec2 avg_diff = c.center - r.center;
			float total_diff = std::abs(avg_diff.x) + std::abs(avg_diff.y);
			if (total_diff != 0.0f) {
				avg_diff.x = avg_diff.x / total_diff;
				avg_diff.y = avg_diff.y / total_diff;
			}

			//get the desired distance between centers and adjust the avg_diff as needed:
			float desired_dist;
			if (axis == 0) {
				if (avg_diff.x < 0.0f)
					avg_diff.x = -1.0f;
				else
					avg_diff.x = 1.0f;
				avg_diff.y = 0.0f;
				desired_dist = c.radius + (r.width / 2.0f);
			}
			else if (axis == 1) {
				avg_diff.x = 0.0f;
				if (avg_diff.y < 0.0f)
					avg_diff.y = -1.0f;
				else
					avg_diff.y = 1.0f;
				desired_dist = c.radius + (r.height / 2.0f);
			}
			else {
				if (avg_diff.x > avg_diff.y)
					desired_dist = c.radius + (r.width / 2.0f);
				else if (avg_diff.x < avg_diff.y)
					desired_dist = c.radius + (r.height / 2.0f);
				else
					desired_dist = c.radius + r.corner_dist;
			}

			//get the vector that points between r's center and the desired position for c's center:
			float vector_from_r_x;
			float vector_from_r_y;
			if (avg_diff.x == 0.0f) {
				vector_from_r_x = 0.0f;
				vector_from_r_y = desired_dist;
			}
			else if (avg_diff.y == 0.0f) {
				vector_from_r_x = desired_dist;
				vector_from_r_y = 0.0f;
			}
			else {
				float ratio_avg_diff = avg_diff.y / avg_diff.x;
				vector_from_r_x = (desired_dist * ratio_avg_diff) / sqrt(1.0f + std::pow(ratio_avg_diff, 2));
				vector_from_r_y = vector_from_r_x / ratio_avg_diff;
			}
			glm::vec2 vector_from_r = glm::vec2(vector_from_r_x, vector_from_r_y);

			//I was asked to return how much the first object would have to move to be out of the second object, but I could just return desired_center
			glm::vec2 desired_center = r.center + vector_from_r;
			return desired_center - c.center;
		}
	}

	//no collision found:
	return glm::vec2(0.0f, 0.0f);
}

//one rectangle colliding with one other of a different shape:

//check for collision between one rectangle and one circle:
bool Collisions::circle_rectangle_collision(Shapes::Rectangle r, Shapes::Circle c) {
	return circle_rectangle_collision(c, r);
}

//check for collision between one rectangle and one triangle:
bool Collisions::rectangle_triangle_collision(Shapes::Rectangle r, Shapes::Triangle t) {
	//if at least one of the points of the triangle is inside the rectangle, then there is a collision:
	for (glm::vec2 point : t.points) {
		if (point.x < r.corners[0].x && point.x > r.corners[1].x &&
			point.y < r.corners[0].y && point.y > r.corners[2].y)
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

//check for collision between one rectangle and one circle:
glm::vec2 Collisions::circle_rectangle_collision(Shapes::Rectangle r, Shapes::Circle c, int axis) {
	//get the distance between the centers of the shapes:
	float center_dist = std::sqrt(std::pow(c.center.x - r.center.x, 2) + std::pow(c.center.y - r.center.y, 2));

	//if the circle radius and rectangle corner distance are too large, there is no chance for a collision:
	if (center_dist > c.radius + r.corner_dist)
		return glm::vec2(0.0f, 0.0f);

	/*if the center of the circle is contained by the rectangle, then there is a collision:
	if (c.center.x < r.corners[0].x && c.center.x > r.corners[1].x &&
		c.center.y < r.corners[0].y && c.center.y > r.corners[2].y) {

		//get the adjusted distance vector between the centers:
		glm::vec2 avg_diff = r.center - c.center;
		float total_diff = std::abs(avg_diff.x) + std::abs(avg_diff.y);
		if (total_diff != 0.0f) {
			avg_diff.x = avg_diff.x / total_diff;
			avg_diff.y = avg_diff.y / total_diff;
		}

		//get the desired distance between centers:
		float desired_dist;
		if (avg_diff.x > avg_diff.y)
			desired_dist = c.radius + (r.width / 2.0f);
		else if (avg_diff.x < avg_diff.y)
			desired_dist = c.radius + (r.height / 2.0f);
		else
			desired_dist = c.radius + r.corner_dist;

		//get the vector that points between r's center and the desired position for c's center:
		float vector_from_c_x;
		float vector_from_c_y;
		if (avg_diff.x == 0.0f) {
			vector_from_c_x = 0.0f;
			vector_from_c_y = desired_dist;
		}
		else if (avg_diff.y == 0.0f) {
			vector_from_c_x = desired_dist;
			vector_from_c_y = 0.0f;
		}
		else {
			float ratio_avg_diff = avg_diff.y / avg_diff.x;
			vector_from_c_x = (desired_dist * ratio_avg_diff) / sqrt(1.0f + std::pow(ratio_avg_diff, 2));
			vector_from_c_y = vector_from_c_x / ratio_avg_diff;
		}
		glm::vec2 vector_from_c = glm::vec2(vector_from_c_x, vector_from_c_y);

		//I was asked to return how much the first object would have to move to be out of the second object, but I could just return desired_center
		glm::vec2 desired_center = c.center + vector_from_c;
		return desired_center - r.center;
	}*/

	//get the points of intersection for the edges of the rectangle and the line made by the shape centers:
	std::vector< glm::vec2 > intersections;
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[0], r.corners[1]));
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[1], r.corners[2]));
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[2], r.corners[3]));
	intersections.emplace_back(line_intersect(c.center, r.center, r.corners[3], r.corners[0]));

	//check for collision:
	for (glm::vec2 intersect : intersections) {
		float intersect_dist = std::sqrt(std::pow(intersect.x - r.center.x, 2) + std::pow(intersect.y - r.center.y, 2));
		if (center_dist < c.radius + intersect_dist) {

			//get the adjusted distance vector between the centers:
			glm::vec2 avg_diff = r.center - c.center;
			float total_diff = std::abs(avg_diff.x) + std::abs(avg_diff.y);
			if (total_diff != 0.0f) {
				avg_diff.x = avg_diff.x / total_diff;
				avg_diff.y = avg_diff.y / total_diff;
			}

			//get the desired distance between centers and adjust the avg_diff as needed:
			float desired_dist;
			if (axis == 0) {
				if (avg_diff.x < 0.0f)
					avg_diff.x = -1.0f;
				else
					avg_diff.x = 1.0f;
				avg_diff.y = 0.0f;
				desired_dist = c.radius + (r.width / 2.0f);
			}
			else if (axis == 1) {
				avg_diff.x = 0.0f;
				if (avg_diff.y < 0.0f)
					avg_diff.y = -1.0f;
				else
					avg_diff.y = 1.0f;
				desired_dist = c.radius + (r.height / 2.0f);
			}
			else {
				if (avg_diff.x > avg_diff.y)
					desired_dist = c.radius + (r.width / 2.0f);
				else if (avg_diff.x < avg_diff.y)
					desired_dist = c.radius + (r.height / 2.0f);
				else
					desired_dist = c.radius + r.corner_dist;
			}

			//get the vector that points between r's center and the desired position for c's center:
			float vector_from_c_x;
			float vector_from_c_y;
			if (avg_diff.x == 0.0f) {
				vector_from_c_x = 0.0f;
				vector_from_c_y = desired_dist;
			}
			else if (avg_diff.y == 0.0f) {
				vector_from_c_x = desired_dist;
				vector_from_c_y = 0.0f;
			}
			else {
				float ratio_avg_diff = avg_diff.y / avg_diff.x;
				vector_from_c_x = (desired_dist * ratio_avg_diff) / sqrt(1.0f + std::pow(ratio_avg_diff, 2));
				vector_from_c_y = vector_from_c_x / ratio_avg_diff;
			}
			glm::vec2 vector_from_c = glm::vec2(vector_from_c_x, vector_from_c_y);

			//I was asked to return how much the first object would have to move to be out of the second object, but I could just return desired_center
			glm::vec2 desired_center = c.center + vector_from_c;
			return desired_center - r.center;
		}
	}

	//no collision found:
	return glm::vec2(0.0f, 0.0f);
}

//one triangle colliding with one other of a different shape:

//check for collision betwwen one triangle and one circle:
bool Collisions::circle_triangle_collision(Shapes::Triangle t, Shapes::Circle c) {
	return circle_triangle_collision(c, t);
}

//check for collision between one triangle and one rectangle:
bool Collisions::rectangle_triangle_collision(Shapes::Triangle t, Shapes::Rectangle r) {
	return rectangle_triangle_collision(r, t);
}

//one player (assumed to be a rectangle) colliding with vectors of other shapes:

//return a vector of all the passed in circles currently colliding with the player (assumed to be a rectangle):
std::vector< Shapes::Circle > Collisions::player_circles_collision(Shapes::Rectangle player, std::vector < Shapes::Circle > circles) {
	//create a vector for the collisions:
	std::vector< Shapes::Circle > hit_circles;

	//check for collisions:
	for (Shapes::Circle circle : circles) {
		if (circle_rectangle_collision(circle, player))
			hit_circles.emplace_back(circle);
	}

	//return a vector containing all of the collisions:
	return hit_circles;
}

//return a vector of all the passed in rectangles currently colliding with the player (assumed to be a rectangle):
std::vector< Shapes::Rectangle > Collisions::player_rectangles_collision(Shapes::Rectangle player, std::vector < Shapes::Rectangle > rectangles) {
	//create a vector for the collisions:
	std::vector< Shapes::Rectangle > hit_rectangles;

	//check for collisions:
	for (Shapes::Rectangle rectangle : rectangles) {
		if (rectangle_rectangle_collision(player, rectangle))
			hit_rectangles.emplace_back(rectangle);
	}

	//return a vector containing all of the collisions:
	return hit_rectangles;
}

//return a vector of all the passed in triangles currently colliding with the player(assumed to be a rectangle) :
std::vector< Shapes::Triangle > Collisions::player_triangles_collision(Shapes::Rectangle player, std::vector < Shapes::Triangle > triangles) {
	//create a vector for the collisions:
	std::vector< Shapes::Triangle > hit_triangles;

	//check for collisions:
	for (Shapes::Triangle triangle : triangles) {
		if (rectangle_triangle_collision(player, triangle))
			hit_triangles.emplace_back(triangle);
	}

	//return a vector containing all of the collisions:
	return hit_triangles;
}

//one player (assumed to be a rectangle) with a new position they want to move to colliding vectors of other shapes:

//check if one player (assumed to be a rectangle) with a new position collides with any circle in the list:
bool Collisions::player_circles_collision(Shapes::Rectangle player, glm::vec2 new_pos, std::vector < Shapes::Circle > circles) {
	//create new rectangle for new position:
	Shapes::Rectangle new_player_pos = Shapes::Rectangle(new_pos, player.width, player.height, false);

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
	Shapes::Rectangle new_player_pos = Shapes::Rectangle(new_pos, player.width, player.height, false);

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
	Shapes::Rectangle new_player_pos = Shapes::Rectangle(new_pos, player.width, player.height, false);

	//check for collisions:
	for (Shapes::Triangle triangle : triangles) {
		if (rectangle_triangle_collision(new_player_pos, triangle))
			return true;
	}

	//no collisions found:
	return false;
}

//functions to help with detecting collisions:

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
		//the lines are parallel (represented by a point with the square root of half the maximum float value for x and y)
		return glm::vec2(std::sqrt(std::numeric_limits<float>::max() / 2.0f), std::sqrt(std::numeric_limits<float>::max() / 2.0f));
	}
	else {
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
