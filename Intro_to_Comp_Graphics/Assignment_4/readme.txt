CSCI3260 HW4 Bezier Curve and Surface

Name: Janic Moser 
Student ID: 1155210428

Note: The screenshots for each part are contained in the screenshots folder.

Write-up:

**Part 1: Casteljau Algorithm**

The basic idea behind the Casteljau Algorithm is to split a Bezier curve into two Bezier curves and then repeat the splitting recursively. The polygon formed by connecting the splitting points approaches the Bezier curve. For the implementation, we calculate the splitting point by recursively determining the intermediate control points until we are left with a single one. This single point represents the splitting point of the Bezier curve. Calculating intermediate control points involves determining a point on the line between two consecutive control points. The position is determined by the parameter t, and by iterating t from 0 to 1, we obtain the Bezier curve.

**Part 2: Bezier Surfaces**

We can extend the usage of the Casteljau Algorithm for Bezier surfaces as well. Consider fixing (u,v) within the range (0,0) to (1,1). In a first step, we compute the splitting point for all control point groups independently with parameter u as t. We can then use these splitting points, with v as the parameter t, to calculate another splitting point that finally lies on the Bezier surface. The Bezier surface is obtained by iterating (u,v) from (0,0) to (1,1). The implementation divides these tasks into three parts: "evaluateStep" calculates one iteration of the Casteljau algorithm; "evaluate1D" computes the splitting point from given control points; and "evaluate" calculates the point on the Bezier surface with parameters (u,v) by first calculating the splitting points for all control point groups with u as t and then computing the splitting point of these intermediate points with v as t.

**Part 3: Shading**

With increasing mesh density, shading becomes a lot more realistic. An issue with the shading is the visibility of triangles, making the surface appear patchy. This is particularly noticeable in the wavy cube. We could improve our shading by implementing a per-vertex normal. As described in the specifications, we can compute this normal as the weighted average of the surface normals of triangles adjacent to the vertex, leading to an improvement in shading.