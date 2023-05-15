
#include "Branchlets.h"

Branchlets::Branchlets(const MPoint& startPoint, int sides, const std::vector<BSegment>& branchSegments, float vOffset) {

	this->sides = sides;

	this->addOne(startPoint, branchSegments, vOffset);
}

BranchletStrips::BranchletStrips(const MPoint& startPoint, const std::vector<BSegment>& stripSegments, float vOffset) {

	this->addOne(startPoint, stripSegments, vOffset);
}

void Branchlets::addOne(const MPoint& startPoint, const std::vector<BSegment>& branchSegments, float vOffset) {

	const int segmentCount = static_cast<int>(branchSegments.size());
	const int initialVertCount = verts.length();
	const int initialUVCount = us.length();

	makeVertexCoords(startPoint, branchSegments, sides);
	makeFaceConnects(segmentCount, sides, initialVertCount);
	makeFaceCounts(segmentCount, sides);
	makeUVs(branchSegments, segmentCount, sides, 1., vOffset);
	makeUVConnects(sides, segmentCount, initialUVCount);
}

void BranchletStrips::addOne(const MPoint& startPoint, const std::vector<BSegment>& stripSegments, float vOffset) {

	int segmentCount = static_cast<int>(stripSegments.size());
	const int initialVertCount = verts.length();
	const int initialUVCount = us.length();

	makeVertexCoords(startPoint, stripSegments, 2);
	makeFaceConnects(segmentCount, initialVertCount);
	makeFaceCounts(segmentCount);
	makeUVs(stripSegments, segmentCount, 1., vOffset);
	makeUVConnects(segmentCount, initialUVCount);
}

Branchlets BranchletCreator::createDefault(int sides) {

	if (sides > 2)
		return Branchlets(sides);
	else if (sides == 2)
		return BranchletStrips();
	else {
		MGlobal::displayWarning(MString() + "Cannot create Branchlets with less than 2 sides");
		return Branchlets();
	}
}

Branchlets BranchletCreator::create(const MPoint& startPoint, int sides, const std::vector<BSegment>& segments, float vOffset) {

	if (sides > 2)
		return Branchlets(startPoint, sides, segments, vOffset);
	else if (sides == 2)
		return BranchletStrips(startPoint, segments, vOffset);
	else {
		MGlobal::displayWarning(MString() + "Cannot create Branchlets with less than 2 sides");
		return Branchlets();
	}
}

void Branchlets::makeVertexCoords(const MPoint startPoint, const std::vector<BSegment>& segs, const int sides) {

	MPoint ringCenter = startPoint;
	MVector major, minor;

	// Make the first ring of vertices
	findEllipseVectors(major, minor, segs[0]);
	makeVertexRing(major, minor, ringCenter, sides, verts);
	ringCenter += segs[0].v;

	// Make all vertex rings between the top and bottom of the branchlet
	for (int i = 0; i < static_cast<int>(segs.size()) - 1; i++) {

		findEllipseVectors(major, minor, ringCenter, segs[i], segs[i + 1]);
		makeVertexRing(major, minor, ringCenter, sides, verts);
		ringCenter += segs[i + 1].v;
	}

	// Make the last ring of vertices
	findEllipseVectors(major, minor, segs.back());
	makeVertexRing(major, minor, ringCenter, sides, verts);

	// Make the cap vertex
	verts.append(ringCenter + (segs.back().v.normal() * segs.back().r));
}

void Branchlets::findEllipseVectors(MVector& major, MVector& minor, BSegment seg) {

	MVector cp = seg.v ^ MVector(-1., 0., 0.);
	if (cp.length() < .001) // In case the axis is directly on the x-axis
		cp = seg.v ^ MVector(0., 1., 0.);

	minor = cp.normal() * seg.r;

	MQuaternion rotation(-(PI / 2.f), minor);
	major = (seg.v.normal() * seg.r).rotateBy(rotation);
}

void Branchlets::findEllipseVectors(MVector& major, MVector& minor, const MPoint& center, const BSegment& bottomSeg, const BSegment& topSeg) {

	double B = bottomSeg.v.angle(topSeg.v); // the angle between the two segments, remember it's measured as if they both face out from the same start point

	double d = std::cos((PI / 2.) - B) * topSeg.v.length();

	if (d < bottomSeg.r * 1.1) { // topSeg is too short or angle is too extreme to intersect

		findEllipseVectors(major, minor, BSegment(bottomSeg.v + topSeg.v, topSeg.r));
		return;
	}

	// The vector along the minor axis is easy since it is always the cross product with its magnitude set to the radius of the top segment
	minor = (topSeg.v ^ bottomSeg.v).normal() * topSeg.r;

	// To find the vector along the major axis we can use trig on the oblique triangle formed by some key points that lie on the plane
	// to the cross product of the two segments.  See ConnectorRing.jpg for details.

	// Find points p and q
	double amountToRotate = (PI / 2.) * -1.;
	MQuaternion rotation(amountToRotate, minor);
	MPoint p = (center - bottomSeg.v) + (bottomSeg.v.normal() * bottomSeg.r).rotateBy(rotation);
	MPoint q = center + (topSeg.v.normal() * topSeg.r).rotateBy(rotation);

	// Find a second angle of the triangle
	MVector pToQ = (q - p);
	double A = PI - (bottomSeg.v.angle(q - p) + B);

	// Use the law of sines to find the distance from p, along the bottom segment's vector, to a point of maximum curvature.
	double a = pToQ.length() * (std::sin(A) / std::sin(B));

	MPoint r = p + (bottomSeg.v.normal() * a);
	major = r - center;
}

void Branchlets::makeVertexRing(const MVector& major, const MVector& minor, const MPoint& center, int sides, MFloatPointArray& outArray) {

	// We can use the parametric equation of an ellipse to calculate the vertices. That is: p(t) = c + cos(t)u + sin(t)v, where p is the vertex
	// coordinate as a function of t, the world space polar angle about the center of the ellipse.

	// Major and minor are the vectors from the center of the ellipse to the points of maximum and minimum curvature.
	// Since these vectors' polar angles change from one segment to the next, and since the resulting vertex depends on 
	// these angles, we need to offset the starting angle accordingly.
	double polarOffset = findVectorPolar(major.x, major.z);

	double angle = 0. - polarOffset;
	double angleIncrement = ((PI * 2.f) / sides) * -1.;

	for (int i = 0; i < sides; i++) {

		MPoint vertex = center + (std::cos(angle) * major) + (std::sin(angle) * minor);
		outArray.append(vertex);

		MPoint vect = vertex - center;

		angle += angleIncrement;
	}
}

void Branchlets::makeFaceConnects(const int segmentCount, const int sides, const int initialVertCount) {

	// For each 4 sided face added we have to specify the indices of the verts that make up its 4 corners - these indices are the faceConnects
	// For each face, they start on the lower left and move counter-clockwise
	for (int i = 0; i < segmentCount; i++) {

		const int firstCornerIndex = (i * sides) + initialVertCount;
		int nextCornerIndex = firstCornerIndex;

		for (int j = 0; j < sides - 1; j++) {

			faceConnects.append(nextCornerIndex);
			faceConnects.append(nextCornerIndex + 1);
			faceConnects.append(nextCornerIndex + 1 + sides);
			faceConnects.append(nextCornerIndex + sides);

			nextCornerIndex++;
		}

		//The pattern for faceConnects is a bit different for the last side in every ring of sides
		faceConnects.append(nextCornerIndex);
		faceConnects.append(firstCornerIndex);
		faceConnects.append(firstCornerIndex + sides);
		faceConnects.append(nextCornerIndex + sides);
	}

	// Finally add the face connects for the triangles connecting the last vertex ring and the cap vertex
	const int firstCornerIndex = (segmentCount * sides) + initialVertCount;
	const int capVertIndex = firstCornerIndex + sides;
	int nextCornerIndex = firstCornerIndex;

	for (int i = 0; i < sides - 1; i++) {

		faceConnects.append(nextCornerIndex);
		faceConnects.append(nextCornerIndex + 1);
		faceConnects.append(capVertIndex);

		nextCornerIndex++;
	}

	faceConnects.append(nextCornerIndex);
	faceConnects.append(firstCornerIndex);
	faceConnects.append(capVertIndex);
}

void BranchletStrips::makeFaceConnects(const int segmentCount, const int initialVertCount) {

	int nextCornerIndex;

	for (int i = 0; i < segmentCount; i++) {

		nextCornerIndex = (i * 2) + initialVertCount;
		faceConnects.append(nextCornerIndex);
		faceConnects.append(nextCornerIndex + 1);
		faceConnects.append(nextCornerIndex + 1 + 2);
		faceConnects.append(nextCornerIndex + 2);
	}

	nextCornerIndex = (segmentCount * 2) + initialVertCount;
	faceConnects.append(nextCornerIndex);
	faceConnects.append(nextCornerIndex + 1);
	faceConnects.append(nextCornerIndex + 2);
}

void Branchlets::makeFaceCounts(const int segmentCount, const int sides) {

	for (int i = 0; i < segmentCount * sides; i++)
		faceCounts.append(4);

	for (int i = 0; i < sides; i++)
		faceCounts.append(3);
}

void BranchletStrips::makeFaceCounts(const int segmentCount) {

	for (int i = 0; i < segmentCount; i++)
		faceCounts.append(4);

	faceCounts.append(3);
}

// This scales uv's so that all u's are within the 0-1 space on the uv map.  v's will match the scaling, so because the mesh is
// a long tube, they may be well outside of 0-1 space
void Branchlets::makeUVs(const std::vector<BSegment>& segs, int segmentCount, int sides, float uWidthMultiplier, float vOffset)
{
	float uFaceWidth = (1.f / sides) * uWidthMultiplier;
	float vScaler = getVScaler(segs[0].r, uFaceWidth, sides, 1.);

	int segmentIndex = 0;
	int sideInd = 0;

	// Assuming there is always a single cap vert, we loop through all but that cap vert and calculate its uv's separately
	for (int vertInd = 0; vertInd < (segmentCount + 1) * sides; vertInd++) {

		us.append(sideInd * uFaceWidth);

		if (segmentIndex > 0) {

			// The index difference between vertices on adjacent rings is equal to sides, while the index difference between UVs on adjacent rings
			// is equal to sides + 1, because we are adding one uv for each ring due to the vertical seam
			float distToVertBelow = verts[vertInd].distanceTo(verts[vertInd - sides]);

			vs.append(vs[vs.length() - (sides + 1)] + (distToVertBelow * vScaler));
		}
		else {

			vs.append(vOffset);
		}

		sideInd++;

		if (sideInd == sides) {

			// Since this is a cylinder with a single vertical seam, we will have one additional uv per ring
			us.append(sideInd * uFaceWidth);
			if (segmentIndex > 0) {

				// the v value is the same as the v on the uv on the opposite side of the 0-1 space
				vs.append(vs[vs.length() - sides]);
			}
			else {

				vs.append(vOffset);
			}

			sideInd = 0;
			segmentIndex++;

			// As we go up the mesh, the rings' radii tend to shrink, so we should adjust the vScaler so that the texture
			// doesn't appear too smushed (this is inevitable on faces between different sized rings)
			// The texture itself will shrink farther up the branch, but at least it will do so uniformly
			// Note that we could adjust the uFaceWidth as well, which would keep the texture the same size for the whole mesh,
			// however this would make the seam visible - pick your poison
			if (segmentIndex < segmentCount)
				vScaler = getVScaler(segs[segmentIndex].r, uFaceWidth, sides, 1.);
		}
	}

	// Now we need a uv for the cap vert for each top face

	// The distance in maya to the cap vert from the ring below it is radius * sqrt(2), because we add 1 radius length when placing
	// the cap vert, so an isosceles triangle is formed with the two equal sides forming a right angle.  Note however that this is
	// only completely accurate if the mesh is perfectly cylindrical (i.e. infinite sides) - so what we have is an approximation
	float vLengthToCap = vScaler * segs.back().r * std::sqrtf(2.f);

	for (int i = 0; i < sides; i++) {

		us.append((i * uFaceWidth) + (uFaceWidth * .5f));

		// Take the average v value of the two lower v's to account for any skew
		float vBetweenLowerUVs = (vs[vs.length() - (sides + 1)] + vs[(vs.length() - (sides + 1)) + 1]) / 2.f;
		vs.append(vBetweenLowerUVs + vLengthToCap);
	}
}

void BranchletStrips::makeUVs(const std::vector<BSegment>& segs, int segmentCount, float uWidthMultiplier, float vOffset)
{
	float uFaceWidth = uWidthMultiplier;
	float vScaler = getVScaler(segs[0].r, uFaceWidth, 2, 1.);

	// To simplify the upcoming loop, make the first two uv's here
	us.append(0.); vs.append(vOffset);
	us.append(uFaceWidth); vs.append(vOffset);

	for (int segmentIndex = 1; segmentIndex < segmentCount + 1; segmentIndex++) {

		// For a strip, all even indexed u's will be 0., and all odds will be uFaceWidth.
		us.append(0.);
		us.append(uFaceWidth);

		if (segmentIndex < segmentCount)
			vScaler = getVScaler(segs[segmentIndex].r, uFaceWidth, 2, 1.);
		else
			vScaler = getVScaler(segs[segmentIndex - 1].r, uFaceWidth, 2, 1.);

		int vertInd = segmentIndex * 2;
		float distToVertBelow = verts[vertInd].distanceTo(verts[vertInd - 2]);
		vs.append(vs[vs.length() - 2] + (distToVertBelow * vScaler));
		vertInd++;
		distToVertBelow = verts[vertInd].distanceTo(verts[vertInd - 2]);
		vs.append(vs[vs.length() - 2] + (distToVertBelow * vScaler));
	}

	float vLengthToCap = vScaler * segs.back().r * std::sqrtf(2.);

	us.append(uFaceWidth * .5f);

	float vBetweenLowerUVs = (vs[vs.length() - 2] + vs[vs.length() - 1]) / 2.f;
	vs.append(vBetweenLowerUVs + vLengthToCap);
}

float Branchlets::getVScaler(float vertRingRadius, float uFaceWidth, int sides, float textureWtoHRatio) {

	float wedgeAngle = (PI * 2.f) / sides;
	float radSqu = vertRingRadius * vertRingRadius;
	float faceWidthInMaya = std::sqrtf((radSqu + radSqu) - (2.f * radSqu * std::cosf(wedgeAngle)));//law of cosines
	return (uFaceWidth / faceWidthInMaya) * textureWtoHRatio;
}

void Branchlets::makeUVConnects(const int sides, const int segmentCount, const int initialUVCount) {

	// This works much like creating faceConnects, except we have one additional column of uv's due to the vertical seam,
	// meaning the uv's don't wrap around. So the code is the same for each face

	int uvSides = sides + 1;
	int uvLowerLeftCornerIndex = initialUVCount;

	for (int i = 0; i < segmentCount; i++) {

		for (int j = 0; j < sides; j++) {

			uvConnects.append(uvLowerLeftCornerIndex);
			uvConnects.append(uvLowerLeftCornerIndex + 1);
			uvConnects.append(uvLowerLeftCornerIndex + 1 + uvSides);
			uvConnects.append(uvLowerLeftCornerIndex + uvSides);

			uvLowerLeftCornerIndex++;
		}

		uvLowerLeftCornerIndex++;
	}

	for (int i = 0; i < sides; i++) {

		uvConnects.append(uvLowerLeftCornerIndex);
		uvConnects.append(uvLowerLeftCornerIndex + 1);
		uvConnects.append(uvLowerLeftCornerIndex + uvSides);

		uvLowerLeftCornerIndex++;
	}
}

void BranchletStrips::makeUVConnects(const int segmentCount, const int initialUVCount) {

	// This works much like creating faceConnects, except we are doing it for the whole mesh in one go. Also
	// we have one additional column of uv's due to the vertical seam, meaning the uv's don't wrap around so
	// the code is the same for each face

	int uvLowerLeftCornerIndex = initialUVCount;

	for (int i = 0; i < segmentCount; i++) {

		uvConnects.append(uvLowerLeftCornerIndex);
		uvConnects.append(uvLowerLeftCornerIndex + 1);
		uvConnects.append(uvLowerLeftCornerIndex + 3);
		uvConnects.append(uvLowerLeftCornerIndex + 2);

		uvLowerLeftCornerIndex += 2;
	}

	uvConnects.append(uvLowerLeftCornerIndex);
	uvConnects.append(uvLowerLeftCornerIndex + 1);
	uvConnects.append(uvLowerLeftCornerIndex + 2);
}

const float Branchlets::PI = 3.1415926f;

double Branchlets::findVectorPolar(double x, double z)
{
	double pol;

	if (x > 0. && z > 0.) { pol = std::atan(z / x); }
	else if (x < 0. && z > 0.) { pol = PI - std::atan(z / std::fabs(x)); }
	else if (x < 0. && z < 0.) { pol = PI + std::atan(std::fabs(z) / std::fabs(x)); }
	else if (x > 0. && z < 0.) { pol = (PI * 2.) - std::atan(std::fabs(z) / x); }
	else if (z == 0. && x > 0.) { pol = 0.; }
	else if (z == 0. && x < 0.) { pol = PI; }
	else if (x == 0. && z > 0.) { pol = (PI / 2.); }
	else if (x == 0. && z < 0.) { pol = PI + (PI / 2.); }
	else { pol = 0.; }

	return pol;

}