#include "pch.h"
#include "SkinnedData.h"
#include "MeshGeometry.h"
#include "MathHelper.h"

using namespace DirectX;

Keyframe::Keyframe()
	: TimePos(0.0f),
	vqs()
{
}

Keyframe::~Keyframe()
{
}

float BoneAnimation::GetStartTime()const
{
	// Keyframes are sorted by time, so first keyframe gives start time.
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime()const
{
	// Keyframes are sorted by time, so last keyframe gives end time.
	float f = Keyframes.back().TimePos;

	return f;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M)const
{

	if (t <= Keyframes.front().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().vqs.Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.front().vqs.Vector);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().vqs.Quat.vec);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else if (t >= Keyframes.back().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.back().vqs.Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.back().vqs.Vector);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.back().vqs.Quat.vec);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else
	{
		for (UINT i = 0; i < Keyframes.size() - 1; ++i)
		{
			if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

		
				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].vqs.Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i + 1].vqs.Scale);

				XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].vqs.Vector);
				XMVECTOR p1 = XMLoadFloat3(&Keyframes[i + 1].vqs.Vector);

				Quaternion q0 = Keyframes[i].vqs.Quat;
				Quaternion q1 = Keyframes[i + 1].vqs.Quat;

				XMVECTOR V = XMVectorLerp(p0, p1, lerpPercent);
				Quaternion Q = Quaternion::Slerp(q0, q1, lerpPercent);
				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q.vec, V));

				break;
			}
		}
	}
}

float AnimationClip::GetClipStartTime()const
{
	// Find smallest start time over all bones in this clip.
	float t = MathHelper::Infinity;
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Min(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime()const
{
	// Find largest end time over all bones in this clip.
	float t = 0.0f;
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Max(t, BoneAnimations[i].GetEndTime());
	}

	return t;
}

void AnimationClip::Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms)const
{
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

float SkinnedData::GetClipStartTime(const std::string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipStartTime();
}

float SkinnedData::GetClipEndTime(const std::string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipEndTime();
}

UINT SkinnedData::BoneCount()const
{
	return mBoneHierarchy.size();
}

void SkinnedData::Set(std::vector<int>& boneHierarchy,
	std::vector<XMFLOAT4X4>& boneOffsets,
	std::map<std::string, AnimationClip>& animations)
{
	mBoneHierarchy = boneHierarchy;
	mBoneOffsets = boneOffsets;
	mAnimations = animations;

	GenerateIKData();
}

void SkinnedData::GenerateIKData()
{
	int i = m_EndEffectorIndex;

	while (i != 0) //0 is the root bone
	{
		m_IKBoneList.push_back(i);

		i = mBoneHierarchy[i];
	}
}

#include "imgui.h"
#include <algorithm>
#include "StepTimer.h"

extern DX::StepTimer g_Timer;
using namespace SimpleMath;

void SkinnedData::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<XMFLOAT4X4>& finalTransforms,
	std::vector<DirectX::XMFLOAT4>& bonePositions, bool doCCD,
	DirectX::XMFLOAT3 target) const
{
	UINT numBones = mBoneOffsets.size();

	static std::vector<XMFLOAT4X4> toParentTransforms(numBones);

	//stop playing animation if doCCD
	if (!doCCD)
	{
		// Interpolate all the bones of this clip at the given time instance.
		auto clip = mAnimations.find(clipName);
		clip->second.Interpolate(timePos, toParentTransforms);
	}
	
	std::vector<XMFLOAT4X4> toRootTransformsTemp(numBones);

	// The root bone has index 0.  The root bone has no parent, so its toRootTransform
	// is just its local bone transform.
	toRootTransformsTemp[0] = toParentTransforms[0];

	// Now find the toRootTransform of the children.
	for (UINT i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = mBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransformsTemp[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransformsTemp[i], toRoot);
	}

	if (doCCD)
	{
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransformsTemp[m_EndEffectorIndex]);
		XMVECTOR endEffectorPos = XMVectorSet(0, 0, 0, 1);
		endEffectorPos = XMVector3TransformCoord(endEffectorPos, toRoot);

		float dist = XMVectorGetX(XMVector3Length(endEffectorPos - target));

		if (dist > 1.0f)
		{

			for (int i = 0; i < m_IKBoneList.size(); ++i)
			{

				//to end effector's local space
				Matrix toEndEffector = XMMatrixIdentity();
				for (int j = i; j >= 0; --j)
				{
					Matrix bone = toParentTransforms[m_IKBoneList[j]];
					toEndEffector = bone * toEndEffector;
				}

				Vector3 vck;
				XMVECTOR scale, rotate, translate;
				XMMatrixDecompose(&scale, &rotate, &translate, toEndEffector);
				vck = translate;


				int parentIndex = mBoneHierarchy[m_IKBoneList[i]];
				XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransformsTemp[parentIndex]);
				XMMATRIX rootToCurrent = XMMatrixInverse(nullptr, parentToRoot);

				Vector3 vdk = XMVector3TransformCoord(XMLoadFloat3(&target), rootToCurrent);

				vck.Normalize();
				vdk.Normalize();

				float cos = vck.Dot(vdk);
				float angle = acosf(std::clamp(cos, -1.0f, 1.0f));

				Vector3 axis = vck.Cross(vdk);
				axis.Normalize();

				XMMATRIX rotation = toEndEffector * XMMatrixRotationAxis(axis, angle * g_Timer.GetElapsedSeconds() * 0.5f);

				//Transform back to local space of current bone
				for (int j = 0; j < i; ++j)
				{
					Matrix bone = toParentTransforms[m_IKBoneList[j]];
					rotation = XMMatrixInverse(nullptr, bone) * rotation;
				}


				XMStoreFloat4x4(&toParentTransforms[m_IKBoneList[i]], rotation);
			}

		}

	}

	//
	// Traverse the hierarchy and transform all the bones to the root space.
	//

	std::vector<XMFLOAT4X4> toRootTransforms(numBones);

	// The root bone has index 0.  The root bone has no parent, so its toRootTransform
	// is just its local bone transform.
	toRootTransforms[0] = toParentTransforms[0];

	// Now find the toRootTransform of the children.
	for (UINT i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);

		int parentIndex = mBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	// Premultiply by the bone offset transform to get the final transform.
	for (UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		
		XMMATRIX finalMatrix = XMMatrixMultiply(offset, toRoot);
		XMStoreFloat4x4(&finalTransforms[i], finalMatrix);
		
		XMVECTOR BonePos = XMVectorSet(0, 0, 0, 1);
		BonePos = XMVector3TransformCoord(BonePos, toRoot);

		XMStoreFloat4(&bonePositions[i], BonePos);
	}


}