#include "Vector.h"

Vector2::Vector2(float x, float y) {
	this->x = x;
	this->y = y;
}

bool Vector2::operator == (const Vector2& vec) {
	if (vec.x == x && vec.y == y)	return true;
	return false;
}
Vector2 Vector2::operator + (Vector2 vec) {
	vec.x = x + vec.x;
	vec.y = y + vec.y;
	return vec;
}
Vector2 Vector2::operator - (Vector2 vec) {
	vec.x = x - vec.x;
	vec.y = y - vec.y;
	return vec;
}
Vector2 Vector2::operator += (const Vector2& vec) {
	x += vec.x;
	y += vec.y;
	return *this;
}
Vector2 Vector2::operator -= (const Vector2& vec) {
	x -= vec.x;
	y -= vec.y;
	return *this;
}
float Vector2::size() {
	return sqrt(x * x + y * y);
}
Vector2 Vector2::normalization() {
	float s = size();
	Vector2 vec(x / s, y / s);
	return vec;
}
Vector2 Vector2::projection(Vector2 standardVector)
{
	standardVector = standardVector.normalization();
	return dot(standardVector) * standardVector;
}
float Vector2::dot(const Vector2& vec)
{
	return (x * vec.x + y * vec.y);
}
float Vector2::angleBetweenVector(Vector2& vec)
{
	float dotProduct = dot(vec);
	float sizeProduct = size() * vec.size();
	return RadianToDegree((acos(dotProduct / sizeProduct)));
}

Vector2 Vector2::operator *= (float f) {
	x *= f;
	y *= f;
	return *this;
}
Vector2 Vector2::operator /= (float f) {
	x /= f;
	y /= f;
	return *this;
}
Vector2 Vector2::operator * (float f) {
	Vector2 vec = *this;
	vec.x *= f;
	vec.y *= f;
	return vec;
}
Vector2 Vector2::operator / (float f) {
	Vector2 vec = *this;
	vec.x /= f;
	vec.y /= f;
	return vec;
}
Vector2 operator * (float f, Vector2 vec) {
	vec.x *= f;
	vec.y *= f;
	return vec;
}
Vector2 operator / (float f, Vector2 vec) {
	vec.x = f / vec.x;
	vec.y = f / vec.y;
	return vec;
}

Vector2 Vector2::direction(float angle) {
	Vector2 vec(cos(DegreeToRadian(angle)), sin(DegreeToRadian(angle)));
	return vec;
}
float Vector2::angle(const Vector2& direction) {
	return RadianToDegree(atan2(direction.y, direction.x));
}

const char* Vector2::toString() {
	char* name = new char[100];
	sprintf_s(name, 100, "Vector2(%.2f, %.2f)", x, y);
	return name;
}

Vector3::Vector3(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

bool Vector3::operator == (const Vector3& vec) {
	if (vec.x == x && vec.y == y && vec.z == z)	return true;
	return false;
}
bool Vector4::operator == (const Vector4& vec) {
	if (vec.x == x && vec.y == y && vec.z == z && vec.w == w)	return true;
	return false;
}
Vector3 Vector3::operator += (const Vector3& vec) {
	x += vec.x;
	y += vec.y;
	z += vec.z;
	return *this;
}
Vector4 Vector4::operator += (const Vector4& vec) {
	x += vec.x;
	y += vec.y;
	z += vec.z;
	w += vec.w;
	return *this;
}
Vector3 Vector3::operator + (Vector3 vec) {
	vec.x += x;
	vec.y += y;
	vec.z += z;
	return vec;
}
Vector4 Vector4::operator + (Vector4 vec) {
	vec.x += x;
	vec.y += y;
	vec.z += z;
	vec.w += w;
	return vec;
}
Vector3 Vector3::operator -= (const Vector3& vec) {
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	return *this;
}
Vector4 Vector4::operator -= (const Vector4& vec) {
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	w -= vec.w;
	return *this;
}
Vector3 Vector3::operator - (Vector3 vec) {
	vec.x = x - vec.x;
	vec.y = y - vec.y;
	vec.z = z - vec.z;
	return vec;
}
Vector4 Vector4::operator - (Vector4 vec) {
	vec.x = x - vec.x;
	vec.y = y - vec.y;
	vec.z = z - vec.z;
	vec.w = w - vec.w;
	return vec;
}
float Vector3::size() {
	return sqrt(x * x + y * y + z * z);
}
float Vector4::size() {
	return sqrt(x * x + y * y + z * z + w * w);
}
float Vector3::dot(const Vector3& vec) {
	return (x * vec.x + y * vec.y + z * vec.z);
}
float Vector3::angleBetweenVector(Vector3& vec)
{
	float dotProduct = dot(vec);
	float sizeProduct = size() * vec.size();
	return RadianToDegree((acos(dotProduct / sizeProduct)));
}
Vector3 Vector3::normalization() {
	float s = size();
	Vector3 vec(x / s, y / s, z / s);
	return vec;
}
Vector3 Vector3::projection(Vector3 standardVector) {
	standardVector = standardVector.normalization();
	return dot(standardVector) * standardVector;
}
Vector3 Vector3::cross(const Vector3& vector)
{
	Vector3 vec;
	vec.x = y * vector.z - z * vector.y;
	vec.y = z * vector.x - x * vector.z;
	vec.z = x * vector.y - y * vector.x;
	return vec;
}
Vector3 Vector3::operator *= (float f) {
	x *= f;
	y *= f;
	z *= f;
	return *this;
}
Vector4 Vector4::operator *= (float f) {
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}
Vector3 Vector3::operator /= (float f) {
	x /= f;
	y /= f;
	z /= f;
	return *this;
}
Vector4 Vector4::operator /= (float f) {
	x /= f;
	y /= f;
	z /= f;
	w /= f;
	return *this;
}
Vector3 Vector3::operator * (float f) {
	Vector3 vec = *this;
	vec.x *= f;
	vec.y *= f;
	vec.z *= f;
	return vec;
}
Vector4 Vector4::operator * (float f) {
	Vector4 vec = *this;
	vec.x *= f;
	vec.y *= f;
	vec.z *= f;
	vec.w *= f;
	return vec;
}
Vector3 Vector3::operator / (float f) {
	Vector3 vec = *this;
	vec.x /= f;
	vec.y /= f;
	vec.z /= f;
	return vec;
}
Vector4 Vector4::operator / (float f) {
	Vector4 vec = *this;
	vec.x /= f;
	vec.y /= f;
	vec.z /= f;
	vec.w /= f;
	return vec;
}


Vector3 operator * (float f, Vector3 vec) {
	vec.x *= f;
	vec.y *= f;
	vec.z *= f;
	return vec;
}
Vector3 operator / (float f, Vector3 vec) {
	vec.x = f / vec.x;
	vec.y = f / vec.y;
	vec.z = f / vec.z;
	return vec;
}
Vector4 operator * (float f, Vector4 vec) {
	vec.x *= f;
	vec.y *= f;
	vec.z *= f;
	vec.w *= f;
	return vec;
}
Vector4 operator / (float f, Vector4 vec) {
	vec.x = f / vec.x;
	vec.y = f / vec.y;
	vec.z = f / vec.z;
	vec.w = f / vec.w;
	return vec;
}

const char* Vector3::toString() {
	char* name = new char[100];
	sprintf_s(name, 100, "Vector3(%.2f, %.2f, %.2f)", x, y, z);
	return name;
}
const char* Vector4::toString() {
	char* name = new char[100];
	sprintf_s(name, 100, "Vector4(%.2f, %.2f, %.2f, %.2f)", x, y, z, w);
	return name;
}