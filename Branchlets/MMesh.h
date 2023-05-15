#pragma once

#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>
#include <maya/MGlobal.h>

// Base class for computing and storing data needed to create meshes using Maya's MFnMesh::create()
class MMesh {

protected:

	MFloatPointArray verts;
	MIntArray faceConnects;
	MIntArray faceCounts;
	MFloatArray us, vs;
	MIntArray uvConnects;

public:

	MMesh() {}

	// Pass all member variables as arguments to MFnMesh::create()
	MStatus createMesh(std::string name) const;
};
