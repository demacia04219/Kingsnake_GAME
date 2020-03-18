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

class Vector2D
{

public:
	float x, y;

	Vector2D(float x = 0, float y = 0);

	float		size();
	float		dot(const Vector2D& vec);
	float		angleBetweenVector(Vector2D& vec);
	Vector2D	normalization();
	Vector2D	projection(Vector2D standardVector);

	bool		operator == (const Vector2D& vec);
	Vector2D	operator += (const Vector2D& vec);
	Vector2D	operator -= (const Vector2D& vec);
	Vector2D	operator *= (float f);
	Vector2D	operator + (Vector2D vec);
	Vector2D	operator - (Vector2D vec);
	Vector2D	operator * (float f);

	const char*	toString();

	static Vector2D		direction(float angle);
	static float		angle(const Vector2D& direction);
};

Vector2D& operator * (float f, Vector2D vec);

class Vector3D
{
public:
	float x, y, z;

	Vector3D(float x = 0, float y = 0, float z = 0);

	float		size();
	float		dot(const Vector3D& vec);
	float		angleBetweenVector(Vector3D& vec);
	Vector3D	normalization();
	Vector3D	projection(Vector3D standardVector);
	
	bool		operator == (const Vector3D& vec);
	Vector3D	operator += (const Vector3D& vec);
	Vector3D	operator -= (const Vector3D& vec);
	Vector3D	operator *= (float f);
	Vector3D	operator + (Vector3D vec);
	Vector3D	operator - (Vector3D vec);
	Vector3D	operator * (float f);
	Vector3D	operator * (const Vector3D& vec);

	const char* toString();
};

Vector3D& operator * (float f, Vector3D vec);


#endif
