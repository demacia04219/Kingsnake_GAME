#include "Vector.h"

Vector2D::Vector2D(float x, float y) {
	this->x = x;
	this->y = y;
}

bool Vector2D::operator == (const Vector2D& vec) {
	if (vec.x == x && vec.y == y)	return true;
	return false;
}
Vector2D Vector2D::operator + (Vector2D vec) {
	vec.x = x + vec.x;
	vec.y = y + vec.y;
	return vec;
}
Vector2D Vector2D::operator - (Vector2D vec) {
	vec.x = x - vec.x;
	vec.y = y - vec.y;
	return vec;
}
Vector2D Vector2D::operator += (const Vector2D& vec) {
	x += vec.x;
	y += vec.y;
	return *this;
}
Vector2D Vector2D::operator -= (const Vector2D& vec) {
	x -= vec.x;
	y -= vec.y;
	return *this;
}
float Vector2D::size() {
	return sqrt(x * x + y * y);
}
Vector2D Vector2D::normalization() {
	float s = size();
	Vector2D vec(x / s, y / s);
	return vec;
}
Vector2D Vector2D::projection(Vector2D standardVector)
{
	standardVector = standardVector.normalization();
	return dot(standardVector) * standardVector;
}
float Vector2D::dot(const Vector2D& vec)
{
	return (x * vec.x + y * vec.y);
}
float Vector2D::angleBetweenVector(Vector2D& vec)
{
	float dotProduct = dot(vec);
	float sizeProduct = size() * vec.size();
	return RadianToDegree((acos(dotProduct / sizeProduct)));
}

Vector2D Vector2D::operator *= (float f) {
	x *= f;
	y *= f;
	return *this;
}
Vector2D Vector2D::operator * (float f) {
	Vector2D vec = *this;
	vec.x *= f;
	vec.y *= f;
	return vec;
}
Vector2D& operator * (float f, Vector2D vec) {
	vec.x *= f;
	vec.y *= f;
	return vec;
}

Vector2D Vector2D::direction(float angle) {
	Vector2D vec(cos(DegreeToRadian(angle)), sin(DegreeToRadian(angle)));
	return vec;
}
float Vector2D::angle(const Vector2D& direction) {
	return RadianToDegree(atan2(direction.y, direction.x));
}

const char* Vector2D::toString() {
	char* name = new char[100];
	sprintf_s(name, 100, "Vector2D(%.2f, %.2f)", x, y);
	return name;
}

Vector3D::Vector3D(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

bool Vector3D::operator == (const Vector3D& vec) {
	if (vec.x == x && vec.y == y && vec.z == z)	return true;
	return false;
}
Vector3D Vector3D::operator += (const Vector3D& vec) {
	x += vec.x;
	y += vec.y;
	z += vec.z;
	return *this;
}
Vector3D Vector3D::operator + (Vector3D vec) {
	vec.x += x;
	vec.y += y;
	vec.z += z;
	return vec;
}
Vector3D Vector3D::operator -= (const Vector3D& vec) {
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	return *this;
}
Vector3D Vector3D::operator - (Vector3D vec) {
	vec.x = x - vec.x;
	vec.y = y - vec.y;
	vec.z = z - vec.z;
	return vec;
}
float Vector3D::size() {
	return sqrt(x * x + y * y + z * z);
}
float Vector3D::dot(const Vector3D& vec) {
	return (x * vec.x + y * vec.y + z * vec.z);
}
float Vector3D::angleBetweenVector(Vector3D& vec)
{
	float dotProduct = dot(vec);
	float sizeProduct = size() * vec.size();
	return RadianToDegree((acos(dotProduct / sizeProduct)));
}
Vector3D Vector3D::normalization() {
	float s = size();
	Vector3D vec(x / s, y / s, z / s);
	return vec;
}
Vector3D Vector3D::projection(Vector3D standardVector) {
	standardVector = standardVector.normalization();
	return dot(standardVector) * standardVector;
}
Vector3D Vector3D::operator *= (float f) {
	x *= f;
	y *= f;
	z *= f;
	return *this;
}
Vector3D Vector3D::operator * (const Vector3D& vec) {
	Vector3D r(y * vec.z - z * vec.y,
		z * vec.x - x * vec.z,
		x * vec.y - y * vec.x);
	return r;
}
Vector3D Vector3D::operator * (float f) {
	Vector3D vec = *this;
	vec.x *= f;
	vec.y *= f;
	vec.z *= f;
	return vec;
}


Vector3D& operator * (float f, Vector3D vec) {
	vec.x *= f;
	vec.y *= f;
	vec.z *= f;
	return vec;
}

const char* Vector3D::toString() {
	char* name = new char[100];
	sprintf_s(name, 100, "Vector3D(%.2f, %.2f, %.2f)", x, y, z);
	return name;
}