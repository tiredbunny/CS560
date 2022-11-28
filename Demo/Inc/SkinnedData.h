#pragma once

#include <DirectXMath.h>
#include <map>
#include <vector>
#include <Windows.h>
#include <string>
#include "Quaternion.h"

struct VQS
{
	VQS() :
		Vector(0.0f, 0.0f, 0.0f),
		Scale(1.0f, 1.0f, 1.0f)
	{
		
	}
	DirectX::XMFLOAT3 Vector;
	DirectX::XMFLOAT3 Scale; //always uniform
	Quaternion Quat;
};


///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
/// 
struct Keyframe
{
	Keyframe();
	~Keyframe();

	float TimePos;

	VQS vqs;
};

///<summary>
/// A BoneAnimation is defined by a list of keyframes.  For time
/// values inbetween two keyframes, we interpolate between the
/// two nearest keyframes that bound the time.  
///
/// We assume an animation always has two keyframes.
///</summary>
struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

	void Interpolate(float t, DirectX::XMFLOAT4X4& M)const;

	std::vector<Keyframe> Keyframes;

};

///<summary>
/// An AnimationClip requires a BoneAnimation for every bone to form
/// the animation clip.    
///</summary>
struct AnimationClip
{
	float GetClipStartTime()const;
	float GetClipEndTime()const;

	void Interpolate(float t, std::vector<DirectX::XMFLOAT4X4>& boneTransforms)const;

	std::vector<BoneAnimation> BoneAnimations;
};

class SkinnedData
{
public:
	SkinnedData() :
		m_EndEffectorIndex(18) //18th is the thumb bone
	{

	}
	UINT BoneCount()const;

	float GetClipStartTime(const std::string& clipName)const;
	float GetClipEndTime(const std::string& clipName)const;

	void GenerateIKData();

	void Set(
		std::vector<int>& boneHierarchy,
		std::vector<DirectX::XMFLOAT4X4>& boneOffsets,
		std::map<std::string, AnimationClip>& animations);

	void GetFinalTransforms(
		const std::string& clipName, float timePos,
		std::vector<DirectX::XMFLOAT4X4>& finalTransforms,
		std::vector<DirectX::XMFLOAT4>& bonePositions,
		bool doCCD = false,
		DirectX::XMFLOAT3 target = DirectX::XMFLOAT3()
	) const;

public:
	// Gives parentIndex of ith bone.
	std::vector<int> mBoneHierarchy;

	std::vector<DirectX::XMFLOAT4X4> mBoneOffsets;

	std::map<std::string, AnimationClip> mAnimations;

	int m_EndEffectorIndex;
	std::vector<int> m_IKBoneList;
};
