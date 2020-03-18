#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <stdio.h>

#ifndef VECTOR_MATH
#define VECTOR_MATH
	#define PI					3.1415926535
	#define DegreeToRadian(x)	(x * PI / 180)
	#define RadianToDegree(x)	(x * 180 / PI)
#endif

class Vector2
{

public:
	float x, y;

	Vector2(float x = 0, float y = 0);

	float	size();
	float	dot(const Vector2& vec);
	float	angleBetweenVector(Vector2& vec);
	Vector2	normalization();
	Vector2	projection(Vector2 standardVector);

	bool	operator == (const Vector2& vec);
	Vector2	operator += (const Vector2& vec);
	Vector2	operator -= (const Vector2& vec);
	Vector2	operator *= (float f);
	Vector2	operator /= (float f);
	Vector2	operator + (Vector2 vec);
	Vector2	operator - (Vector2 vec);
	Vector2	operator * (float f);
	Vector2	operator / (float f);

	const char*	toString();

	static Vector2		direction(float angle);
	static float		angle(const Vector2& direction);
};

Vector2 operator * (float f, Vector2 vec);
Vector2 operator / (float f, Vector2 vec);

class Vector3
{
public:
	float x, y, z;

	Vector3(float x = 0, float y = 0, float z = 0);

	float	size();
	float	dot(const Vector3& vec);
	float	angleBetweenVector(Vector3& vec);
	Vector3 cross(const Vector3& vector);
	Vector3	normalization();
	Vector3	projection(Vector3 standardVector);
	
	bool	operator == (const Vector3& vec);
	Vector3	operator += (const Vector3& vec);
	Vector3	operator -= (const Vector3& vec);
	Vector3	operator *= (float f);
	Vector3	operator /= (float f);
	Vector3	operator + (Vector3 vec);
	Vector3	operator - (Vector3 vec);
	Vector3	operator * (float f);
	Vector3	operator / (float f);

	const char* toString();
};

Vector3 operator * (float f, Vector3 vec);
Vector3 operator / (float f, Vector3 vec);


class Vector4
{
public:
	float x, y, z, w;

	Vector4(float x = 0, float y = 0, float z = 0, float w = 0);

	float	size();
	
	bool	operator == (const Vector4& vec);
	Vector4	operator += (const Vector4& vec);
	Vector4	operator -= (const Vector4& vec);
	Vector4	operator *= (float f);
	Vector4	operator /= (float f);
	Vector4	operator + (Vector4 vec);
	Vector4	operator - (Vector4 vec);
	Vector4	operator * (float f);
	Vector4	operator / (float f);

	const char* toString();
};

Vector4 operator * (float f, Vector4 vec);
Vector4 operator / (float f, Vector4 vec);


#endif
