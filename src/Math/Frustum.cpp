#include "Math/Frustum.h"
#include "Math/Matrix4x4.h"
#include <cmath> // pour sqrtf

// Normalise un plan (longueur =1)
// Nécessaire pour que la distance calculée soit correcte
static void NormalizePlane(Plane &plane)
{
    float mag = plane.normal.Magnitude();
    if (mag > 0.0f)
    {
        plane.normal.x /= mag;
        plane.normal.y /= mag;
        plane.normal.z /= mag;
        plane.distance /= mag;
    }
}

void Frustum::Update(const Matrix4x4 &viewProjectionMatrix)
{
    const float *m = viewProjectionMatrix.m;

    // Extraction des 6 plans de la matrice vue-projection
    // Chaque plan est défini par A,B,C,D
    // La normale du plan est (A B C) et la distance D.

    // Plan Gauche
    m_planes[0].normal.x = m[3] + m[0];
    m_planes[0].normal.y = m[7] + m[4];
    m_planes[0].normal.z = m[11] + m[8];
    m_planes[0].distance = m[15] + m[12];
    NormalizePlane(m_planes[0]);

    // Plan Droit
    m_planes[1].normal.x = m[3] - m[0];
    m_planes[1].normal.y = m[7] - m[4];
    m_planes[1].normal.z = m[11] - m[8];
    m_planes[1].distance = m[15] - m[12];
    NormalizePlane(m_planes[1]);

    // Plan Bas
    m_planes[2].normal.x = m[3] + m[1];
    m_planes[2].normal.y = m[7] + m[5];
    m_planes[2].normal.z = m[11] + m[9];
    m_planes[2].distance = m[15] + m[13];
    NormalizePlane(m_planes[2]);

    // Plan Haut
    m_planes[3].normal.x = m[3] - m[1];
    m_planes[3].normal.y = m[7] - m[5];
    m_planes[3].normal.z = m[11] - m[9];
    m_planes[3].distance = m[15] - m[13];
    NormalizePlane(m_planes[3]);

    // Plan Près
    m_planes[4].normal.x = m[3] + m[2];
    m_planes[4].normal.y = m[7] + m[6];
    m_planes[4].normal.z = m[11] + m[10];
    m_planes[4].distance = m[15] + m[14];
    NormalizePlane(m_planes[4]);

    // Plan Loin
    m_planes[5].normal.x = m[3] - m[2];
    m_planes[5].normal.y = m[7] - m[6];
    m_planes[5].normal.z = m[11] - m[10];
    m_planes[5].distance = m[15] - m[14];
    NormalizePlane(m_planes[5]);
}

bool Frustum::Intersects(const AABB &aabb) const
{
    // On teste l'AABB contre chaque plan du frustum
    for (int i = 0; i < 6; ++i)
    {

        const Plane &plane = m_planes[i];
        /* Pour chaque plan, on doit trouver le point de l'AABB qui est "le plus loin"
        dans la direction opposée à la normale du plan (le "coin négatif".)
        Si ce même point est du côté "extérieur" du plan (distance positive), alors la
        boîte entière est à l'extérieur, et il n'y a pas d'intersection.
        */
        Vector3 pVertex; // Le "p-vertex" (positive) est le coin le plus dans la direction de la normale
                         // Nous avons besoin du "n-vertex" (négatif) ici

        // Si la composante de la normale est positive, le coin négatif est sur "min"
        // Si elle est négative, il est sur "max"
        pVertex.x = (plane.normal.x > 0) ? aabb.max.x : aabb.min.x;
        pVertex.y = (plane.normal.y > 0) ? aabb.max.y : aabb.min.y;
        pVertex.z = (plane.normal.z > 0) ? aabb.max.z : aabb.min.z;

        // On calcule la distance de ce coin au plan.
        if (plane.GetSignedDistanceToPoint(pVertex) < 0)
        {
            // Si la distance est négative, ce "coin le plus positif" est en fait derrière le plan
            // Cela signifie que toute la boîte est derrière le plan
            // On peut donc conclure qu'il n'y a pas d'intersection et s'arrêter là
            return false;
        }
    }
    // Si la boîte n'est entièrement en dehors d'aucun des 6 plans,
    // cela signifie qu'elle est soit entièrement dedans, soit en intersection
    // Dans les deux cas, on return true..
    return true;
}

bool Frustum::IntersectsSphere(const Vector3 &center, float radius) const
{
    // Pour chaque plan du frustum
    for (int i = 0; i < 6; ++i)
    {
        // On calcule la distance signée du centre de la sphère.
        // Si cette distance est inférieure au rayon négatif, cela signifie que la sphère est totalement
        // hors du frustum (côté extérieur du plan)
        if (m_planes[i].GetSignedDistanceToPoint(center) < -radius)
        {
            return false; // Pas d'intersection, on arrête
        }
    }

    // Si elle n'est pas complètement à l'extérieur des 6 plans, elle est partiellement à l'intérieur
    return true;
}