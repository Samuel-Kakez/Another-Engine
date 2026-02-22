#include "Math/AABB.h"
#include "Math/Matrix4x4.h"
#include <limits> // pour std::numeric_limits
#include <vector>
#include <algorithm> // pour std::min et std::max

AABB::AABB() : min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
               max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max())
{
    // Le constructeur initialise une boîte "inversée" ou "invalide"
    // La première fois que Extend() sera appelée, les valeurs min/max
    // prendront directement les coordonnées du premier point.
}

void AABB::Extend(const Vector3 &point)
{
    // Pour chaque axe, on compare le point avec les bornes actuelles
    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    min.z = std::min(min.z, point.z);

    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
    max.z = std::max(max.z, point.z);
}

AABB AABB::Transform(const Matrix4x4 &transform) const
{
    // On ne peut pas seulement transformer min et max, car une rotation pourrait faire que l'ancient point min devient un point max.
    // La seule méthode correcte est de transformer les 8 coins de la boîte et de construire une nouvelle AABB à partir de ces 8 points.

    // 1. Définir les 8 coins de l'AABB locale.
    std::vector<Vector3> corners = {
        Vector3(min.x, min.y, min.z),
        Vector3(max.x, min.y, min.z),
        Vector3(max.x, max.y, min.z),
        Vector3(min.x, max.y, min.z),
        Vector3(min.x, min.y, max.z),
        Vector3(max.x, min.y, max.z),
        Vector3(max.x, max.y, max.z),
        Vector3(min.x, max.y, max.z)};

    // 2. Créer une nouvelle AABB (invalide au départ)
    AABB transformedAABB;

    // 3. Transformer chaque coin et étendre la nouvelle AABB.
    for (const auto &corner : corners)
    {
        // On multiplie le coin par la matrice pour obtenir sa position dans le monde
        Vector3 transformedCorner = transform * corner;
        // On étend la nouvelle AABB pour qu'elle contienne ce coin transformé
        transformedAABB.Extend(transformedCorner);
    }
    return transformedAABB;
}

std::vector<Vector3> AABB::GetCorners() const
{
    // Créé un vecteur et retourne les 8 coins de la boîte
    return {
        Vector3(min.x, min.y, min.z),
        Vector3(max.x, min.y, min.z),
        Vector3(max.x, max.y, min.z),
        Vector3(min.x, max.y, min.z),
        Vector3(min.x, min.y, max.z),
        Vector3(max.x, min.y, max.z),
        Vector3(max.x, max.y, max.z),
        Vector3(min.x, max.y, max.z)};
}

bool AABB::IntersectsSphere(const Vector3 &sphereCenter, float sphereRadius) const
{
    // Trouve le point le plus proche sur l'AABB du centre de la sphère
    float closestX = std::max(min.x, std::min(sphereCenter.x, max.x));
    float closestY = std::max(min.y, std::min(sphereCenter.y, max.y));
    float closestZ = std::max(min.z, std::min(sphereCenter.z, max.z));

    // Calcule la distance au carré entre ce point et le centre de la sphère
    float distanceSquared = (closestX - sphereCenter.x) * (closestX - sphereCenter.x) +
                            (closestY - sphereCenter.y) * (closestY - sphereCenter.y) +
                            (closestZ - sphereCenter.z) * (closestZ - sphereCenter.z);

    // Il y a intersection si la distance au carré est inférieure au rayon au carré
    return distanceSquared < (sphereRadius * sphereRadius);
}