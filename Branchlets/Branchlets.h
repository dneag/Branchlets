#pragma once

#include <maya/MVector.h>
#include <maya/MQuaternion.h>
#include <maya/MGlobal.h>

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include "MMesh.h"

// The building block of Branchlets.  A list of these is input to the Branchlet constructor.
struct BSegment {

	// A vector describing the direction and length of a branchlet segment
	MVector v;

	// The radius of the segment
	float r;

	BSegment(const MVector& V, float R) : v(V), r(R) {}
};

// Computes and stores all data needed to create n-sided closed tube-like meshes using Maya's MFnMesh::create()
// Note that this may represent a single branchlet or many branchlets, as more may be added with addOne(),
// however it will always hold a single set of arguments for a single MObject
class Branchlets : public MMesh {

	int sides = 0;

	// Calculate all face connects for a branchlet
	void makeFaceConnects(const int segmentCount, const int sides, const int initialVertCount);

	// Calculate all face counts for a branchlet
	void makeFaceCounts(const int segmentCount, const int sides);

	// Calculate all uv coordintes for a branchlet
	void makeUVs(const std::vector<BSegment>& segs, int segmentCount, int sides, float uWidthMultiplier, float lowestV);

	// Calculate all uv connects for a branchlet
	void makeUVConnects(const int sides, const int segmentCount, const int initialUVCount);

protected:

	const static float PI;

	// Calculate all the vertex coordinates for a branchlet
	void makeVertexCoords(const MPoint startPoint, const std::vector<BSegment>& segs, const int sides);

	// Find the vectors from the center of the circle on the plane whose normal is defined by 'seg' to two points 90 degrees apart along its perimeter
	void findEllipseVectors(MVector& major, MVector& minor, BSegment seg);

	// Find the vectors from the center of the ellipse on the plane whose normal is defined by 'topSeg' to the points of maximum (major) and minimum (minor) curvature
	void findEllipseVectors(MVector& major, MVector& minor, const MPoint& center, const BSegment& bottomSeg, const BSegment& topSeg);

	// Use the parametric equation for an ellipse in 3D space to calculate the coordinates of 'sides' vertices along its perimeter
	void makeVertexRing(const MVector& major, const MVector& minor, const MPoint& center, int sides, const MVector& topSegVect);

	// Find the amount with which to multiply the distance between v coords so that they are proportional to their length in Maya
	float getVScaler(float vertRingRadius, float uFaceWidth, int sides, float textureWtoHRatio);

	// Find the polar angle of a vector in world space
	double findVectorPolar(double x, double z);

	// Project p onto the plane to which q is the normal vector, given a shared starting point
	MVector projectByNormal(const MVector& p, const MVector& q, const MPoint& s);

public:

	Branchlets() {}

	Branchlets(int SIDES) : sides(SIDES) {}

	Branchlets(const MPoint& startPoint, int sides, const std::vector<BSegment>& branchSegments, float vOffset);

	// Appends to the member variables to form another branchlet
	void addOne(const MPoint& startPoint, const std::vector<BSegment>& branchSegments, float vOffset);
};

// Like Branchlets, but with 2 sides
class BranchletStrips : public Branchlets {

	// Calculate all face connects for a strip
	void makeFaceConnects(const int segmentCount, const int initialVertCount);

	// Calculate all face counts for a strip
	void makeFaceCounts(const int segmentCount);

	// Calculate all uv coordintes for a strip
	void makeUVs(const std::vector<BSegment>& segs, int segmentCount, float uWidthMultiplier, float lowestV);

	// Calculate all uv connects for a strip
	void makeUVConnects(const int segmentCount, const int initialUVCount);

public:

	BranchletStrips() {}

	BranchletStrips(const MPoint& startPoint, const std::vector<BSegment>& branchSegments, float vOffset);

	// Appends to the member variables to form another strip
	void addOne(const MPoint& startPoint, const std::vector<BSegment>& branchSegments, float vOffset);
};

class BranchletCreator {

public:

	BranchletCreator() {}

	// Creates a Branchlets or BranchletStrips object depending on the value of sides.  All member variables are empty.
	Branchlets createDefault(int sides);

	// Creates a Branchlets or BranchletStrips object depending on the value of sides.
	Branchlets create(const MPoint& startPoint, int sides, const std::vector<BSegment>& branchSegments, float vOffset);
};
