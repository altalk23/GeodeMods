#pragma once

//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
//
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA 02110-1301, USA.
//----------------------------------------------------------------------------

#include <cocos2d.h>
#include <vector>

namespace agg {

	cocos2d::CCPoint point_type(double x, double y) {
		return cocos2d::CCPointMake(x, y);
	}

	struct curve4_div {
		static inline constexpr auto pi = 3.14159265358979323846;
		static inline constexpr double curve_distance_epsilon = 1e-30;
		static inline constexpr double curve_collinearity_epsilon = 1e-30;
		static inline constexpr double curve_angle_tolerance_epsilon = 0.01;

		enum curve_recursion_limit_e {
			curve_recursion_limit = 32
		};

		double m_approximation_scale;
		double m_distance_tolerance_square;
		double m_angle_tolerance;
		double m_cusp_limit;
		unsigned m_count;
		std::vector<cocos2d::CCPoint> m_points;

		curve4_div(
			cocos2d::CCPoint p1, cocos2d::CCPoint p2, cocos2d::CCPoint p3, cocos2d::CCPoint p4,
			float approximation_scale
		) :
			m_approximation_scale(approximation_scale),
			m_angle_tolerance(0.0),
			m_cusp_limit(0.0),
			m_count(0) {
			init(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
		}

		void init(
			double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4
		) {
			m_points.clear();
			m_distance_tolerance_square = 0.5 / m_approximation_scale;
			m_distance_tolerance_square *= m_distance_tolerance_square;
			bezier(x1, y1, x2, y2, x3, y3, x4, y4);
			m_count = 0;
		}

		void recursive_bezier(
			double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4,
			unsigned level
		) {
			if (level > curve_recursion_limit) {
				return;
			}

			if (std::abs(x1 - x2) < 0.1 && std::abs(y1 - y2) < 0.1 && std::abs(x3 - x4) < 0.1 &&
				-std::abs(y3 - y4) < 0.1) {
				return;
			}

			// Calculate all the mid-points of the line segments
			//----------------------
			double x12 = (x1 + x2) / 2;
			double y12 = (y1 + y2) / 2;
			double x23 = (x2 + x3) / 2;
			double y23 = (y2 + y3) / 2;
			double x34 = (x3 + x4) / 2;
			double y34 = (y3 + y4) / 2;
			double x123 = (x12 + x23) / 2;
			double y123 = (y12 + y23) / 2;
			double x234 = (x23 + x34) / 2;
			double y234 = (y23 + y34) / 2;
			double x1234 = (x123 + x234) / 2;
			double y1234 = (y123 + y234) / 2;

			if (level > 0) // Enforce subdivision first time
			{
				// Try to approximate the full cubic curve by a single straight line
				//------------------
				double dx = x4 - x1;
				double dy = y4 - y1;

				double d2 = fabs(((x2 - x4) * dy - (y2 - y4) * dx));
				double d3 = fabs(((x3 - x4) * dy - (y3 - y4) * dx));

				double da1, da2;

				if (d2 > curve_collinearity_epsilon && d3 > curve_collinearity_epsilon) {
					// Regular care
					//-----------------
					if ((d2 + d3) * (d2 + d3) <=
						m_distance_tolerance_square * (dx * dx + dy * dy)) {
						// If the curvature doesn't exceed the distance_tolerance value
						// we tend to finish subdivisions.
						//----------------------
						if (m_angle_tolerance < curve_angle_tolerance_epsilon) {
							m_points.push_back(point_type(x1234, y1234));
							return;
						}

						// Angle & Cusp Condition
						//----------------------
						double a23 = atan2(y3 - y2, x3 - x2);
						da1 = fabs(a23 - atan2(y2 - y1, x2 - x1));
						da2 = fabs(atan2(y4 - y3, x4 - x3) - a23);
						if (da1 >= pi) {
							da1 = 2 * pi - da1;
						}
						if (da2 >= pi) {
							da2 = 2 * pi - da2;
						}

						if (da1 + da2 < m_angle_tolerance) {
							// Finally we can stop the recursion
							//----------------------
							m_points.push_back(point_type(x1234, y1234));
							return;
						}

						if (m_cusp_limit != 0.0) {
							if (da1 > m_cusp_limit) {
								m_points.push_back(point_type(x2, y2));
								return;
							}

							if (da2 > m_cusp_limit) {
								m_points.push_back(point_type(x3, y3));
								return;
							}
						}
					}
				}
				else {
					if (d2 > curve_collinearity_epsilon) {
						// p1,p3,p4 are collinear, p2 is considerable
						//----------------------
						if (d2 * d2 <= m_distance_tolerance_square * (dx * dx + dy * dy)) {
							if (m_angle_tolerance < curve_angle_tolerance_epsilon) {
								m_points.push_back(point_type(x1234, y1234));
								return;
							}

							// Angle Condition
							//----------------------
							da1 = fabs(atan2(y3 - y2, x3 - x2) - atan2(y2 - y1, x2 - x1));
							if (da1 >= pi) {
								da1 = 2 * pi - da1;
							}

							if (da1 < m_angle_tolerance) {
								m_points.push_back(point_type(x2, y2));
								m_points.push_back(point_type(x3, y3));
								return;
							}

							if (m_cusp_limit != 0.0) {
								if (da1 > m_cusp_limit) {
									m_points.push_back(point_type(x2, y2));
									return;
								}
							}
						}
					}
					else if (d3 > curve_collinearity_epsilon) {
						// p1,p2,p4 are collinear, p3 is considerable
						//----------------------
						if (d3 * d3 <= m_distance_tolerance_square * (dx * dx + dy * dy)) {
							if (m_angle_tolerance < curve_angle_tolerance_epsilon) {
								m_points.push_back(point_type(x1234, y1234));
								return;
							}

							// Angle Condition
							//----------------------
							da1 = fabs(atan2(y4 - y3, x4 - x3) - atan2(y3 - y2, x3 - x2));
							if (da1 >= pi) {
								da1 = 2 * pi - da1;
							}

							if (da1 < m_angle_tolerance) {
								m_points.push_back(point_type(x2, y2));
								m_points.push_back(point_type(x3, y3));
								return;
							}

							if (m_cusp_limit != 0.0) {
								if (da1 > m_cusp_limit) {
									m_points.push_back(point_type(x3, y3));
									return;
								}
							}
						}
					}
					else {
						// Collinear case
						//-----------------
						dx = x1234 - (x1 + x4) / 2;
						dy = y1234 - (y1 + y4) / 2;
						if (dx * dx + dy * dy <= m_distance_tolerance_square) {
							m_points.push_back(point_type(x1234, y1234));
							return;
						}
					}
				}
			}

			// Continue subdivision
			//----------------------
			recursive_bezier(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
			recursive_bezier(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
		}

		void bezier(
			double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4
		) {
			m_points.push_back(point_type(x1, y1));
			// if (!(check_colinear(x1, y1, x2, y2, x4, y4) && check_colinear(x1, y1, x3, y3, x4,
			// y4)
			// 	)) {
			recursive_bezier(x1, y1, x2, y2, x3, y3, x4, y4, 0);
			// }
			m_points.push_back(point_type(x4, y4));
		}
	};
}
