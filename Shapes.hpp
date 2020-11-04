#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Shapes {

	struct Circle {
		Circle(glm::vec2 c, float r);

		glm::vec2 center;
		float radius;
	};

	struct Rectangle {
		Rectangle(glm::vec2 c, float w, float h);

		glm::vec2 center;
		float width;
		float height;

		std::vector< glm::vec2 > corners;
		float corner_dist;
	};

	struct Triangle {
		Triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);

		std::vector< glm::vec2 > points;
	};
}