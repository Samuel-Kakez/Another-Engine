#include "Math/Vector3.h"
#include <cmath>    // pour sqrtf

// constructeurs
// j'utilise la liste d'initialisation de membres
// vecteur3 avec valeurs par défaut
Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f)
{
}

// vecteur3 avec valeurs en entrée
Vector3::Vector3(float initialX, float initialY, float initialZ) : x(initialX), y(initialY), z(initialZ)
{
}

// fonctions membres
void Vector3::Log() const
{

}

// sqrtf signifie qu'elle travaille avec des floats
float Vector3::Magnitude() const
{
    return sqrtf(x * x + y * y + z * z);
}

// pas de const ici, car la fonction modifie l'objet
void Vector3::Normalize()
{
    // étape 1 : calculer la magnitude
    float mag = Magnitude();

    // étape 2 : vérifier si la magnitude est > 0
    if (mag > 0.0f)
    {
        // étape 3 : si c'est le cas, on divise x, y et z par la magnitude
        x /= mag;
        y /= mag;
        z /= mag;
    }
}

// surcharges d'opérateurs
// ces fonctions retournent un nouvel objet Vector3. Elles sont const
Vector3 Vector3::operator+(const Vector3 &other) const
{
    return Vector3(this->x + other.x, this->y + other.y, this->z + other.z);
}

Vector3 Vector3::operator-(const Vector3 &other) const
{
    return Vector3(this->x - other.x, this->y - other.y, this->z - other.z);
}

Vector3 Vector3::operator*(float scalar) const
{
    return Vector3(this->x * scalar, this->y * scalar, this->z * scalar);
}

Vector3 Vector3::operator*(const Vector3 &other) const
{
    return Vector3(x * other.x, y * other.y, z * other.z);
}

// fonction libre
// elle opère sur deux objets fournis en paramètre
Vector3 Cross(const Vector3 &a, const Vector3 &b)
{
    return Vector3(
        (a.y * b.z - a.z * b.y),
        (a.z * b.x - a.x * b.z),
        (a.x * b.y - a.y * b.x));
}

float Vector3::Dot(const Vector3 &other) const
{
    return (x * other.x) + (y * other.y) + (z * other.z);
}