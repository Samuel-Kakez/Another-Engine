#include <cmath>

#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"
#include "Math/Constants.h"

// --- Constructeur ---
// On utilise la liste d'initialisation
Matrix4x4::Matrix4x4() : m{

                             1.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f}
{
}

// --- Fonctions membres ---
void Matrix4x4::Log() const
{

}

// --- Fonctions de création statiques ---
Matrix4x4 Matrix4x4::CreateTranslation(const Vector3 &translation)
{
    // créé une nouvelle matrice
    Matrix4x4 result;

    // modifie les valeurs
    result.m[12] = translation.x;
    result.m[13] = translation.y;
    result.m[14] = translation.z;

    return result;
}

Matrix4x4 Matrix4x4::CreateScale(const Vector3 &scale)
{
    // créé une nouvelle matrice
    Matrix4x4 result;

    // modifie les valeurs
    result.m[0] = scale.x;
    result.m[5] = scale.y;
    result.m[10] = scale.z;

    return result;
}

Matrix4x4 Matrix4x4::CreateRotationX(float angle_radians)
{
    // créé une nouvelle matrice
    Matrix4x4 result;

    // calcule le cosinus et le sinus
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);

    // modifie les valeurs
    result.m[5] = c;
    result.m[6] = s;
    result.m[9] = -s;
    result.m[10] = c;

    return result;
}

Matrix4x4 Matrix4x4::CreateRotationY(float angle_radians)
{
    // créé une nouvelle matrice
    Matrix4x4 result;

    // calcule le cosinus et le sinus
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);

    // modifie les valeurs
    result.m[0] = c;
    result.m[2] = -s;
    result.m[8] = s;
    result.m[10] = c;

    return result;
}

Matrix4x4 Matrix4x4::CreateRotationZ(float angle_radians)
{
    // créé une nouvelle matrice
    Matrix4x4 result;

    // calcule le cosinus et le sinus
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);

    // modifie les valeurs
    result.m[0] = c;
    result.m[1] = s;
    result.m[4] = -s;
    result.m[5] = c;

    return result;
}

Matrix4x4 Matrix4x4::CreateLookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up)
{

    // -- calcul des axes de la caméra --

    // étape 1 : calculer le vecteur 'forward' (f)

    Vector3 f = target - eye;
    f.Normalize();

    // étape 2 : calculer le vecteur 'right' (s)
    Vector3 s = Cross(f, up);
    s.Normalize();

    // étape 3 : calculer le vrai vecteur 'up' de la caméra (u)
    Vector3 u = Cross(s, f);

    Matrix4x4 result;
    // rotation
    result.m[0] = s.x;
    result.m[4] = s.y;
    result.m[8] = s.z;
    result.m[1] = u.x;
    result.m[5] = u.y;
    result.m[9] = u.z;
    result.m[2] = -f.x;
    result.m[6] = -f.y;
    result.m[10] = -f.z;

    // translation avec dot product
    result.m[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    result.m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    result.m[14] = (f.x * eye.x + f.y * eye.y + f.z * eye.z);

    // autres valeurs de la matrice identité intouchées
    result.m[15] = 1.0f;
    result.m[3] = result.m[7] = result.m[11] = 0.0f;

    return result;
}

Matrix4x4 Matrix4x4::CreatePerspectiveProjection(float fov_y_degrees, float aspect_ratio, float near_plane, float far_plane)
{

    // étape 1 : créer une matrice et l'initialiser à zéro
    Matrix4x4 result;
    for (int i = 0; i < 16; i++)
    {
        result.m[i] = 0.0f;
    }

    // étape 2 : convertis le fov en radians
    const float fov_rads = fov_y_degrees * (Math::PI / 180.0);

    // étape 3 : calculer les facteurs d'échelle
    const float tan_half_fov = tanf(fov_rads / 2.0f);
    const float scale_y = 1.0f / tan_half_fov;
    const float scale_x = scale_y / aspect_ratio;

    // étape 4 : remplir les éléments de la matrice
    result.m[0] = scale_x;
    result.m[5] = scale_y;
    result.m[10] = -(far_plane + near_plane) / (far_plane - near_plane);
    result.m[11] = -1.0f;
    result.m[14] = -(2 * far_plane * near_plane) / (far_plane - near_plane);

    return result;
}

Vector3 Matrix4x4::TransformDirection(const Vector3 &dir) const
{
    Vector3 result;
    result.x = dir.x * m[0] + dir.y * m[4] + dir.z * m[8];
    result.y = dir.x * m[1] + dir.y * m[5] + dir.z * m[9];
    result.z = dir.x * m[2] + dir.y * m[6] + dir.z * m[10];
    return result;
}

// --- surcharge d'opérateurs ---
Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &other) const
{
    Matrix4x4 result;
    // boucle sur les colonnes du résultat (0 à 3)
    for (int col = 0; col < 4; ++col)
    {
        // boucle sur les lignes du résultat (0 à 3)
        for (int row = 0; row < 4; ++row)
        {
            // calcule le produit scalaire pour l'élément (row, col)
            float sum = 0.0f;
            // boucle pour le produit scalaire (0 à 3)
            for (int i = 0; i < 4; ++i)
            {
                // élément de la ligne "row" de la matrice gauche (this->m)
                // multiplié par
                // élément de la colonne "col" de la matrice droite (other.m)
                sum += this->m[i * 4 + row] * other.m[col * 4 + i];
            }
            // assigne le résultat à la bonne case de la matrice result
            result.m[col * 4 + row] = sum;
        }
    }

    return result;
}

Vector3 Matrix4x4::operator*(const Vector3 &v) const
{
    Vector3 result;

    result.x = v.x * m[0] + v.y * m[4] + v.z * m[8] + m[12];
    result.y = v.x * m[1] + v.y * m[5] + v.z * m[9] + m[13];
    result.z = v.x * m[2] + v.y * m[6] + v.z * m[10] + m[14];

    return result;
}

Vector3 Matrix4x4::GetTranslation() const
{
    // La translation est stockée dans m12 13 et 14
    return Vector3(m[12], m[13], m[14]);
}

Vector3 Matrix4x4::GetForward() const
{
    // Le vecteur forward (-Z local) correspond à la troisième colonne de la matrice
    return Vector3(-m[8], -m[9], -m[10]);
}

Matrix4x4 Matrix4x4::InverseRigid() const
{
    Matrix4x4 result;

    // La partie supérieure 3x3 de la matrice (rotation) est transposée
    // L'inverse d'une matrice de rotation est sa transposée
    result.m[0] = this->m[0];
    result.m[1] = this->m[4];
    result.m[2] = this->m[8];
    result.m[4] = this->m[1];
    result.m[5] = this->m[5];
    result.m[6] = this->m[9];
    result.m[8] = this->m[2];
    result.m[9] = this->m[6];
    result.m[10] = this->m[10];

    // La nouvelle translation est l'opposé de la translation d'origine, transformée par la rotation transposée
    Vector3 translation(this->m[12], this->m[13], this->m[14]);
    Vector3 newTranslation;
    newTranslation.x = -(translation.x * result.m[0] + translation.y * result.m[4] + translation.z * result.m[8]);
    newTranslation.y = -(translation.x * result.m[1] + translation.y * result.m[5] + translation.z * result.m[9]);
    newTranslation.z = -(translation.x * result.m[2] + translation.y * result.m[6] + translation.z * result.m[10]);

    result.m[12] = newTranslation.x;
    result.m[13] = newTranslation.y;
    result.m[14] = newTranslation.z;

    // La dernière ligne reste [0, 0, 0, 1];
    result.m[3] = 0.0f;
    result.m[7] = 0.0f;
    result.m[11] = 0.0f;
    result.m[15] = 1.0f;

    return result;
}

Matrix4x4 Matrix4x4::Inverse() const
{
    Matrix4x4 result;
    float inv[16], det;

    // étape 1 : Calculer la matrice des cofacteurs 
    // c'est la partie la plus intensive en calcule (1h à écrire D:)
    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    // étape 2 :  Calculer le déterminant en utilisant la première ligne de la matrice originale
    // et la première colonne de la matrice des cofacteurs
    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    // étape 3 : Si le déterminant est zéro, la matrice n'est pas inversible
    if (det == 0)
        return Matrix4x4(); // Matrice identité si non inversible

    // étape 4 : diviser chaque élément de la matrice des cofacteurs par le déterminant
    // c'est l'étape finale pour obtenir l'inverse
    det = 1.0f / det;
    for (int i = 0; i < 16; i++)
        result.m[i] = inv[i] * det;

    return result;
}

Matrix4x4 Matrix4x4::Transpose() const
{
    Matrix4x4 result;
    // échange simplement les lignes et les colonnes
    result.m[0] = m[0];
    result.m[4] = m[1];
    result.m[8] = m[2];
    result.m[12] = m[3];
    result.m[1] = m[4];
    result.m[5] = m[5];
    result.m[9] = m[6];
    result.m[13] = m[7];
    result.m[2] = m[8];
    result.m[6] = m[9];
    result.m[10] = m[10];
    result.m[14] = m[11];
    result.m[3] = m[12];
    result.m[7] = m[13];
    result.m[11] = m[14];
    result.m[15] = m[15];
    return result;
}