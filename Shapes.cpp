#include "Shapes.hpp"
using namespace Shapes;

Shapes::Circle::Circle(glm::vec2 c, float r, bool s) {
	center = c;
	radius = r;
	stationary = s;
}

//NOTE: For now it assumes the first argument of Rectangle is the center, but this can change (maybe to a corner)

Rectangle::Rectangle() {}

Shapes::Rectangle::Rectangle(glm::vec2 c, float w, float h, bool s) {
	center = c;
	width = w;
	height = h;

	corners.emplace_back(center + glm::vec2(width / 2.0f, height / 2.0f));   //top right
	corners.emplace_back(center + glm::vec2(-width / 2.0f, height / 2.0f));  //top left
	corners.emplace_back(center + glm::vec2(-width / 2.0f, -height / 2.0f)); //bottom left
	corners.emplace_back(center + glm::vec2(width / 2.0f, -height / 2.0f));  //bottom right

	corner_dist = std::sqrt(std::pow(width, 2) + std::pow(height, 2));

	stationary = s;
}

Shapes::Triangle::Triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, bool s) {
	points.emplace_back(p1);
	points.emplace_back(p2);
	points.emplace_back(p3);

	stationary = s;
}
