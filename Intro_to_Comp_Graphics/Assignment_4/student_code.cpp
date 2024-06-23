
/*
Type your name and student ID here
    - Name:
    - Student ID:
*/

#include "bezierPatch.h"
#include "bezierCurve.h"

  /**
   * Evaluates one step of the de Casteljau's algorithm using the given points and
   * the scalar parameter t (class member).
   *
   * @param points A vector of points in 2D
   * @return A vector containing intermediate points or the final interpolated vector
   */
  std::vector<glm::vec2> BezierCurve::evaluateStep(std::vector<glm::vec2> const &points)
  { 
    int s = points.size() - 1;
    std::vector<glm::vec2> p;
    for (int i = 0; i < s; i++) {
        p.push_back((1 - t) * points[i] + t * points[i + 1]);
    }

    return p;
  }

  /**
   * Evaluates one step of the de Casteljau's algorithm using the given points and
   * the scalar parameter t (function parameter).
   *
   * @param points    A vector of points in 3D
   * @param t         Scalar interpolation parameter
   * @return A vector containing intermediate points or the final interpolated vector
   */
  std::vector<glm::vec3> BezierPatch::evaluateStep(std::vector<glm::vec3> const &points, float t) const
  {
    int s = points.size() - 1;
    std::vector<glm::vec3> p;
    for (int i = 0; i < s; i++) {
        p.push_back((1 - t) * points[i] + t * points[i + 1]);
    }
    return p;
  }

  /**
   * Fully evaluates de Casteljau's algorithm for a vector of points at scalar parameter t
   *
   * @param points    A vector of points in 3D
   * @param t         Scalar interpolation parameter
   * @return Final interpolated vector
   */
  glm::vec3 BezierPatch::evaluate1D(std::vector<glm::vec3> const &points, float t) const
  {
    std::vector<glm::vec3> p = points;
    while (p.size() > 1) {
        p = evaluateStep(p, t);
    }
    return p.at(0);
  }

  /**
   * Evaluates the Bezier patch at parameter (u, v)
   *
   * @param u         Scalar interpolation parameter
   * @param v         Scalar interpolation parameter (along the other axis)
   * @return Final interpolated vector
   */
  glm::vec3 BezierPatch::evaluate(float u, float v) const 
  {  
    std::vector<std::vector<glm::vec3>> p1 = this->controlPoints;
    std::vector<glm::vec3> p2;
    for (std::vector<glm::vec3> pi : p1) {
        p2.push_back(evaluate1D(pi, u));
    }
    return evaluate1D(p2, v);
  }


void compute_triangle_normal(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& n)
{
  n = glm::cross((p1 - p0), (p2 - p0));
  n = glm::normalize(n);
}


/*glm::vec3 Vertex::normal(void) const
{
  // TODO Part 3.
  // Returns an approximate unit normal at this vertex, computed by
  // taking the area-weighted average of the normals of neighboring
  // triangles, then normalizing.

  return glm::vec3();
}*/
 