#pragma once

#include "SimpleMath.h"

class Quaternion 
{
public:
	DirectX::SimpleMath::Vector4 vec;

public:
	inline double Norm()
	{
		return vec.Length();
	}


	inline void Normalize()
	{
		vec.Normalize();
	}

	inline void Conjugate()
	{
		vec.x = -vec.x;
		vec.y = -vec.y;
		vec.z = -vec.z;
	}

	inline void Inverse()
	{
		double norm = Norm();
		Conjugate();
		vec.x /= norm;
		vec.y /= norm;
		vec.z /= norm;
		vec.w /= norm;
	}

	static Quaternion Slerp(Quaternion qa, Quaternion qb, double t)
	{
		// quaternion to return
		Quaternion qm;
		// Calculate angle between them.
		double cosHalfTheta = qa.vec.w * qb.vec.w + qa.vec.x * qb.vec.x + qa.vec.y * qb.vec.y + qa.vec.z * qb.vec.z;
		// if qa=qb or qa=-qb then theta = 0 and we can return qa
		if (abs(cosHalfTheta) >= 1.0) {
			qm.vec.w = qa.vec.w; qm.vec.x = qa.vec.x; qm.vec.y = qa.vec.y; qm.vec.z = qa.vec.z;
			return qm;
		}
		// Calculate temporary values.
		double halfTheta = acos(cosHalfTheta);
		double sinHalfTheta = sqrt(1.0 - cosHalfTheta * cosHalfTheta);

		if (fabs(sinHalfTheta) < 0.001) 
		{ 
			qm.vec.w = (qa.vec.w * 0.5 + qb.vec.w * 0.5);
			qm.vec.x = (qa.vec.x * 0.5 + qb.vec.x * 0.5);
			qm.vec.y = (qa.vec.y * 0.5 + qb.vec.y * 0.5);
			qm.vec.z = (qa.vec.z * 0.5 + qb.vec.z * 0.5);
			return qm;
		}

		double ratioA = sin((1 - t) * halfTheta) / sinHalfTheta;
		double ratioB = sin(t * halfTheta) / sinHalfTheta;

		qm.vec.w = (qa.vec.w * ratioA + qb.vec.w * ratioB);
		qm.vec.x = (qa.vec.x * ratioA + qb.vec.x * ratioB);
		qm.vec.y = (qa.vec.y * ratioA + qb.vec.y * ratioB);
		qm.vec.z = (qa.vec.z * ratioA + qb.vec.z * ratioB);
		return qm;
	}


};

