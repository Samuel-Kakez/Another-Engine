#include "Math/Plane.h"

float Plane::GetSignedDistanceToPoint(const Vector3 &point) const
{
    // produit scalaire entre la normale et le point qui nous donne la projection
    // du point sur la normale. En ajoutant D, on obtient la distance au plan
    return normal.x * point.x + normal.y * point.y + normal.z * point.z + distance;
}