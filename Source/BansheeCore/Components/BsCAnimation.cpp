//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Components/BsCAnimation.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCRenderable.h"
#include "Components/BsCBone.h"
#include "Mesh/BsMesh.h"
#include "Animation/BsMorphShapes.h"
#include "Animation/BsAnimationClip.h"
#include "Private/RTTI/BsCAnimationRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace std::placeholders;

namespace bs
{
	CAnimation::CAnimation()
		:mWrapMode(AnimWrapMode::Loop), mSpeed(1.0f), mEnableCull(true), mUseBounds(false), mPreviewMode(false)
	{
		mNotifyFlags = TCF_Transform;
		setFlag(ComponentFlag::AlwaysRun, true);

		setName("Animation");
	}

	CAnimation::CAnimation(const HSceneObject& parent)
		: Component(parent), mWrapMode(AnimWrapMode::Loop), mSpeed(1.0f), mEnableCull(true), mUseBounds(false), mPreviewMode(false)
	{
		mNotifyFlags = TCF_Transform;
		setFlag(ComponentFlag::AlwaysRun, true);

		setName("Animation");
	}

	void CAnimation::setDefaultClip(const HAnimationClip& clip)
	{
		mDefaultClip = clip;

		if(clip.isLoaded() && mInternal != nullptr && !mPreviewMode)
			mInternal->play(clip);
	}

	void CAnimation::setWrapMode(AnimWrapMode wrapMode)
	{
		mWrapMode = wrapMode;

		if (mInternal != nullptr && !mPreviewMode)
			mInternal->setWrapMode(wrapMode);
	}

	void CAnimation::setSpeed(float speed)
	{
		mSpeed = speed;

		if (mInternal != nullptr && !mPreviewMode)
			mInternal->setSpeed(speed);
	}

	void CAnimation::play(const HAnimationClip& clip)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->play(clip);
	}

	void CAnimation::blendAdditive(const HAnimationClip& clip, float weight, float fadeLength, UINT32 layer)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->play(clip);
	}

	void CAnimation::blend1D(const Blend1DInfo& info, float t)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->blend1D(info, t);
	}

	void CAnimation::blend2D(const Blend2DInfo& info, const Vector2& t)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->blend2D(info, t);
	}

	void CAnimation::crossFade(const HAnimationClip& clip, float fadeLength)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->crossFade(clip, fadeLength);
	}

	void CAnimation::sample(const HAnimationClip& clip, float time)
	{
		if (mInternal != nullptr)
			mInternal->sample(clip, time);
	}

	void CAnimation::stop(UINT32 layer)
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->stop(layer);
	}

	void CAnimation::stopAll()
	{
		if (mInternal != nullptr && !mPreviewMode)
			mInternal->stopAll();
	}

	bool CAnimation::isPlaying() const
	{
		if (mInternal != nullptr)
			return mInternal->isPlaying();

		return false;
	}

	bool CAnimation::getState(const HAnimationClip& clip, AnimationClipState& state)
	{
		if (mInternal != nullptr)
			return mInternal->getState(clip, state);

		return false;
	}

	void CAnimation::setState(const HAnimationClip& clip, AnimationClipState state)
	{
		if (mInternal != nullptr)
			return mInternal->setState(clip, state);
	}

	void CAnimation::setMorphChannelWeight(const String& name, float weight)
	{
		if (mInternal == nullptr)
			return;

		if (mAnimatedRenderable == nullptr)
			return;

		HMesh mesh = mAnimatedRenderable->getMesh();
		if (!mesh.isLoaded())
			return;

		SPtr<MorphShapes> morphShapes = mesh->getMorphShapes();
		if (morphShapes == nullptr)
			return;

		const Vector<SPtr<MorphChannel>>& channels = morphShapes->getChannels();
		for (UINT32 i = 0; i < (UINT32)channels.size(); i++)
		{
			if (channels[i]->getName() == name)
			{
				mInternal->setMorphChannelWeight(i, weight);
				break;
			}
		}
	}

	void CAnimation::setBounds(const AABox& bounds)
	{
		mBounds = bounds;

		if(mUseBounds)
		{
			if(mAnimatedRenderable != nullptr)
			{
				SPtr<Renderable> renderable = mAnimatedRenderable->_getRenderable();
				if (renderable != nullptr)
					renderable->setOverrideBounds(bounds);

				if(mInternal != nullptr && !mPreviewMode)
				{
					AABox bounds = mBounds;

					bounds.transformAffine(SO()->getWorldMatrix()); 
					mInternal->setBounds(bounds);
				}
			}
		}
	}

	void CAnimation::setUseBounds(bool enable)
	{
		mUseBounds = enable;

		_updateBounds();
	}

	void CAnimation::setEnableCull(bool enable)
	{
		mEnableCull = enable;

		if (mInternal != nullptr && !mPreviewMode)
			mInternal->setCulling(enable);
	}

	UINT32 CAnimation::getNumClips() const
	{
		if (mInternal != nullptr)
			return mInternal->getNumClips();

		return 0;
	}

	HAnimationClip CAnimation::getClip(UINT32 idx) const
	{
		if (mInternal != nullptr)
			return mInternal->getClip(idx);

		return HAnimationClip();
	}

	void CAnimation::onInitialized()
	{
		
	}

	void CAnimation::onDestroyed()
	{
		destroyInternal();
	}

	void CAnimation::onDisabled()
	{
		destroyInternal();
	}

	void CAnimation::onEnabled()
	{
		if(mPreviewMode)
		{
			destroyInternal();
			mPreviewMode = false;
		}
		
		if(SceneManager::instance().isRunning())
			restoreInternal(false);
	}

	void CAnimation::update()
	{
		bool isRunning = SceneManager::instance().isRunning();
		if (mInternal == nullptr || !isRunning)
			return;

		HAnimationClip newPrimaryClip = mInternal->getClip(0);
		if (newPrimaryClip != mPrimaryPlayingClip)
			_refreshClipMappings();

		if (_scriptUpdateFloatProperties)
			_scriptUpdateFloatProperties();
	}

	void CAnimation::onTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->getActive())
			return;

		if ((flags & (TCF_Transform)) != 0)
			_updateBounds(false);
	}

	void CAnimation::restoreInternal(bool previewMode)
	{
		if (mInternal != nullptr)
			destroyInternal();

		mInternal = Animation::create();

		mAnimatedRenderable = SO()->getComponent<CRenderable>();

		if (!previewMode)
		{
			mInternal->onEventTriggered.connect(std::bind(&CAnimation::eventTriggered, this, _1, _2));

			mInternal->setWrapMode(mWrapMode);
			mInternal->setSpeed(mSpeed);
			mInternal->setCulling(mEnableCull);
		}

		_updateBounds();

		if (!previewMode)
		{
			if (mDefaultClip.isLoaded())
				mInternal->play(mDefaultClip);

			mPrimaryPlayingClip = mInternal->getClip(0);
			if (mPrimaryPlayingClip.isLoaded())
			{
				if (_scriptRebuildFloatProperties)
					_scriptRebuildFloatProperties(mPrimaryPlayingClip);
			}
		}

		setBoneMappings();

		if(!previewMode)
			updateSceneObjectMapping();

		if (mAnimatedRenderable != nullptr)
			mAnimatedRenderable->_registerAnimation(mThisHandle);
	}

	void CAnimation::destroyInternal()
	{
		if (mAnimatedRenderable != nullptr)
			mAnimatedRenderable->_unregisterAnimation();

		mPrimaryPlayingClip = nullptr;
		mMappingInfos.clear();

		// This should release the last reference and destroy the internal listener
		mInternal = nullptr;
	}

	bool CAnimation::_togglePreviewMode(bool enabled)
	{
		bool isRunning = SceneManager::instance().isRunning();

		if(enabled)
		{
			// Cannot enable preview while running
			if (isRunning)
				return false;

			restoreInternal(true);
			mPreviewMode = true;

			return true;
		}
		else
		{
			if (!isRunning)
				destroyInternal();

			mPreviewMode = false;
			return false;
		}
	}

	bool CAnimation::_getGenericCurveValue(UINT32 curveIdx, float& value)
	{
		if (mInternal == nullptr)
			return false;

		return mInternal->getGenericCurveValue(curveIdx, value);
	}

	void CAnimation::mapCurveToSceneObject(const String& curve, const HSceneObject& so)
	{
		if (mInternal == nullptr)
			return;

		mInternal->mapCurveToSceneObject(curve, so);
	}

	void CAnimation::unmapSceneObject(const HSceneObject& so)
	{
		if (mInternal == nullptr)
			return;

		mInternal->unmapSceneObject(so);
	}

	void CAnimation::_addBone(const HBone& bone)
	{
		if (mInternal == nullptr)
			return;

		HSceneObject currentSO = bone->SO();

		SceneObjectMappingInfo newMapping;
		newMapping.sceneObject = currentSO;
		newMapping.isMappedToBone = true;
		newMapping.bone = bone;

		mMappingInfos.push_back(newMapping);
		mInternal->mapCurveToSceneObject(bone->getName(), newMapping.sceneObject);
	}

	void CAnimation::_removeBone(const HBone& bone)
	{
		if (mInternal == nullptr)
			return;

		HSceneObject newSO;
		for (UINT32 i = 0; i < (UINT32)mMappingInfos.size(); i++)
		{
			if (mMappingInfos[i].bone == bone)
			{
				mMappingInfos.erase(mMappingInfos.begin() + i);
				mInternal->unmapSceneObject(mMappingInfos[i].sceneObject);
				i--;
			}
		}
	}

	void CAnimation::_notifyBoneChanged(const HBone& bone)
	{
		if (mInternal == nullptr)
			return;

		for (UINT32 i = 0; i < (UINT32)mMappingInfos.size(); i++)
		{
			if (mMappingInfos[i].bone == bone)
			{
				mInternal->unmapSceneObject(mMappingInfos[i].sceneObject);
				mInternal->mapCurveToSceneObject(bone->getName(), mMappingInfos[i].sceneObject);
				break;
			}
		}
	}

	void CAnimation::_registerRenderable(const HRenderable& renderable)
	{
		mAnimatedRenderable = renderable;

		_updateBounds();
	}

	void CAnimation::_unregisterRenderable()
	{
		mAnimatedRenderable = nullptr;
	}

	void CAnimation::_updateBounds(bool updateRenderable)
	{
		SPtr<Renderable> renderable;
		if (updateRenderable && mAnimatedRenderable != nullptr)
			renderable = mAnimatedRenderable->_getRenderable();

		if (mUseBounds)
		{
			if (renderable != nullptr)
			{
				renderable->setUseOverrideBounds(true);
				renderable->setOverrideBounds(mBounds);
			}

			if (mInternal != nullptr)
			{
				AABox bounds = mBounds;
				bounds.transformAffine(SO()->getWorldMatrix());

				mInternal->setBounds(bounds);
			}
		}
		else
		{
			if (renderable != nullptr)
				renderable->setUseOverrideBounds(false);

			if (mInternal != nullptr)
			{
				AABox bounds;
				if (mAnimatedRenderable != nullptr)
					bounds = mAnimatedRenderable->getBounds().getBox();

				mInternal->setBounds(bounds);
			}
		}
	}

	void CAnimation::setBoneMappings()
	{
		mMappingInfos.clear();

		SceneObjectMappingInfo rootMapping;
		rootMapping.sceneObject = SO();
		rootMapping.isMappedToBone = true;

		mMappingInfos.push_back(rootMapping);
		mInternal->mapCurveToSceneObject("", rootMapping.sceneObject);

		Vector<HBone> childBones = findChildBones();
		for (auto& entry : childBones)
			_addBone(entry);
	}

	void CAnimation::updateSceneObjectMapping()
	{
		Vector<SceneObjectMappingInfo> newMappingInfos;
		for(auto& entry : mMappingInfos)
		{
			if (entry.isMappedToBone)
				newMappingInfos.push_back(entry);
			else
				unmapSceneObject(entry.sceneObject);
		}

		if (mPrimaryPlayingClip.isLoaded())
		{
			HSceneObject root = SO();

			const auto& findMappings = [&](const String& name, AnimationCurveFlags flags)
			{
				if (flags.isSet(AnimationCurveFlag::ImportedCurve))
					return;

				HSceneObject currentSO = root->findPath(name);

				bool found = false;
				for (UINT32 i = 0; i < (UINT32)newMappingInfos.size(); i++)
				{
					if (newMappingInfos[i].sceneObject == currentSO)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					SceneObjectMappingInfo newMappingInfo;
					newMappingInfo.isMappedToBone = false;
					newMappingInfo.sceneObject = currentSO;

					newMappingInfos.push_back(newMappingInfo);
					mapCurveToSceneObject(name, currentSO);
				}
			};

			SPtr<AnimationCurves> curves = mPrimaryPlayingClip->getCurves();
			for(auto& curve : curves->position)
				findMappings(curve.name, curve.flags);

			for(auto& curve : curves->rotation)
				findMappings(curve.name, curve.flags);

			for(auto& curve : curves->scale)
				findMappings(curve.name, curve.flags);
		}

		mMappingInfos = newMappingInfos;
	}

	void CAnimation::_refreshClipMappings()
	{
		mPrimaryPlayingClip = mInternal->getClip(0);

		if (_scriptRebuildFloatProperties)
			_scriptRebuildFloatProperties(mPrimaryPlayingClip);

		updateSceneObjectMapping();		
	}

	Vector<HBone> CAnimation::findChildBones()
	{
		Stack<HSceneObject> todo;
		todo.push(SO());

		Vector<HBone> bones;
		while (todo.size() > 0)
		{
			HSceneObject currentSO = todo.top();
			todo.pop();

			HBone bone = currentSO->getComponent<CBone>();
			if (bone != nullptr)
			{
				bone->_setParent(getHandle(), true);
				bones.push_back(bone);
			}

			int childCount = currentSO->getNumChildren();
			for (int i = 0; i < childCount; i++)
			{
				HSceneObject child = currentSO->getChild(i);
				if (child->getComponent<CAnimation>() != nullptr)
					continue;

				todo.push(child);
			}
		}

		return bones;
	}

	void CAnimation::eventTriggered(const HAnimationClip& clip, const String& name)
	{
		onEventTriggered(clip, name);

		if(_scriptOnEventTriggered)
			_scriptOnEventTriggered(clip, name);
	}

	RTTITypeBase* CAnimation::getRTTIStatic()
	{
		return CAnimationRTTI::instance();
	}

	RTTITypeBase* CAnimation::getRTTI() const
	{
		return CAnimation::getRTTIStatic();
	}
}