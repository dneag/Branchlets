# Branchlets

A set of classes for using Autodesk Maya's C++ API to make tree branch meshes, given lists of segments defined by a vector and radius.

This branch uses a different approach to creating vertex rings.  It only uses the ellipse formula for the initial ring.  For all further rings, 
we add a vector, in the direction of the next segment, to each vertex's location.  The challenge is calculating the correct length for each vector.
